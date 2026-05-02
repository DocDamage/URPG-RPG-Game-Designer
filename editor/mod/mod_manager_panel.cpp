#include "editor/mod/mod_manager_panel.h"

#include "engine/core/mod/mod_registry_validator.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace {

std::string issueCategoryToString(urpg::mod::ModIssueCategory category) {
    using urpg::mod::ModIssueCategory;

    switch (category) {
    case ModIssueCategory::EmptyId:
        return "empty_id";
    case ModIssueCategory::EmptyName:
        return "empty_name";
    case ModIssueCategory::EmptyVersion:
        return "empty_version";
    case ModIssueCategory::DuplicateDependency:
        return "duplicate_dependency";
    case ModIssueCategory::SelfDependency:
        return "self_dependency";
    case ModIssueCategory::MissingEntryPoint:
        return "missing_entry_point";
    }

    return "unknown";
}

std::string issueSeverityToString(urpg::mod::ModIssueSeverity severity) {
    return severity == urpg::mod::ModIssueSeverity::Error ? "error" : "warning";
}

std::string boolPolicyToString(bool value) {
    return value ? "allowed" : "denied";
}

std::string hotLoadEventTypeToString(urpg::mod::ModHotLoadEventType type) {
    using urpg::mod::ModHotLoadEventType;

    switch (type) {
    case ModHotLoadEventType::Registered:
        return "registered";
    case ModHotLoadEventType::Changed:
        return "changed";
    case ModHotLoadEventType::Reloaded:
        return "reloaded";
    case ModHotLoadEventType::Failed:
        return "failed";
    case ModHotLoadEventType::MissingEntrypoint:
        return "missing_entrypoint";
    }

    return "unknown";
}

nlohmann::json sandboxPolicyToJson(const urpg::mod::ModSandboxPolicy& policy) {
    return {
        {"file_system_read", boolPolicyToString(policy.allowFileSystemRead)},
        {"file_system_write", boolPolicyToString(policy.allowFileSystemWrite)},
        {"network_access", boolPolicyToString(policy.allowNetworkAccess)},
        {"native_interop", boolPolicyToString(policy.allowNativeInterop)},
    };
}

nlohmann::json hotLoadEventToJson(const urpg::mod::ModHotLoadEvent& event) {
    return {
        {"type", hotLoadEventTypeToString(event.type)},
        {"mod_id", event.modId},
        {"entry_point", event.entryPoint.generic_string()},
        {"message", event.message},
    };
}

} // namespace

namespace urpg::editor {

void ModManagerPanel::bindRegistry(urpg::mod::ModRegistry* registry) {
    registry_ = registry;
}

void ModManagerPanel::bindLoader(urpg::mod::ModLoader* loader) {
    loader_ = loader;
}

void ModManagerPanel::bindHotLoader(urpg::mod::ModHotLoader* hot_loader) {
    hot_loader_ = hot_loader;
}

void ModManagerPanel::bindStoreCatalog(urpg::mod::ModStoreCatalog* store_catalog) {
    store_catalog_ = store_catalog;
}

void ModManagerPanel::bindMarketplaceProviderProfile(
    const urpg::mod::ModMarketplaceProviderProfile* provider_profile) {
    marketplace_provider_profile_ = provider_profile;
}

void ModManagerPanel::render() {
    nlohmann::json snapshot;
    snapshot["registered_count"] = 0;
    snapshot["active_count"] = 0;
    snapshot["resolved_load_order"] = nlohmann::json::array();
    snapshot["cycle_warning"] = nullptr;
    snapshot["registry_bound"] = registry_ != nullptr;
    snapshot["loader_bound"] = loader_ != nullptr;
    snapshot["hot_loader_bound"] = hot_loader_ != nullptr;
    snapshot["store_catalog_bound"] = store_catalog_ != nullptr;
    snapshot["marketplace_provider_profile_bound"] = marketplace_provider_profile_ != nullptr;
    snapshot["marketplace_provider_profile"] =
        marketplace_provider_profile_ ? marketplace_provider_profile_->toJson() : nlohmann::json(nullptr);
    snapshot["marketplace_provider_profile_status"] =
        marketplace_provider_profile_
            ? urpg::release::providerProfileStatusToJson(
                  urpg::mod::modMarketplaceProviderProfileStatus(*marketplace_provider_profile_))
            : nlohmann::json(nullptr);
    const bool actionsEnabled = registry_ != nullptr && loader_ != nullptr;
    const bool hotLoadActionsEnabled = registry_ != nullptr && hot_loader_ != nullptr;
    const bool storeActionsEnabled = loader_ != nullptr && store_catalog_ != nullptr;
    snapshot["status"] = registry_ == nullptr ? "disabled" : "empty";
    snapshot["disabled_reason"] = registry_ == nullptr ? "No mod registry is bound." : "";
    snapshot["empty_reason"] = registry_ == nullptr ? "" : "No mods are registered.";
    snapshot["error_message"] = "";
    snapshot["status_messages"] = nlohmann::json::array();
    if (registry_ == nullptr) {
        snapshot["status_messages"].push_back("No mod registry is bound.");
    }
    if (loader_ == nullptr) {
        snapshot["status_messages"].push_back("No mod loader is bound; mod actions are disabled.");
    }
    if (hot_loader_ == nullptr) {
        snapshot["status_messages"].push_back("No mod hot-loader is bound; live reload event log is disabled.");
    }
    if (store_catalog_ == nullptr) {
        snapshot["status_messages"].push_back("No mod store catalog is bound; store install actions are disabled.");
    }
    snapshot["actions"] = {
        {"import_manifest", actionsEnabled}, {"register_manifest", actionsEnabled},
        {"activate", actionsEnabled},        {"deactivate", actionsEnabled},
        {"reload", actionsEnabled},          {"track_hot_load_mod", hotLoadActionsEnabled},
        {"track_registered_hot_load_mods", hotLoadActionsEnabled},
        {"poll_hot_loader", hotLoadActionsEnabled}, {"clear_hot_load_events", hot_loader_ != nullptr},
        {"install_store_entry", storeActionsEnabled},
    };
    snapshot["last_action"] = last_action_snapshot_.is_null() ? nlohmann::json::object() : last_action_snapshot_;
    snapshot["hot_load"] = {
        {"bound", hot_loader_ != nullptr},
        {"event_count", 0},
        {"events", nlohmann::json::array()},
        {"last_poll_any_reloaded", last_hot_poll_result_.anyReloaded},
        {"last_poll_event_count", last_hot_poll_result_.events.size()},
        {"last_poll_events", nlohmann::json::array()},
    };

    for (const auto& event : last_hot_poll_result_.events) {
        snapshot["hot_load"]["last_poll_events"].push_back(hotLoadEventToJson(event));
    }

    if (hot_loader_) {
        const auto& eventLog = hot_loader_->eventLog();
        snapshot["hot_load"]["event_count"] = eventLog.size();
        for (const auto& event : eventLog) {
            snapshot["hot_load"]["events"].push_back(hotLoadEventToJson(event));
        }
    }
    snapshot["store"] = {
        {"bound", store_catalog_ != nullptr},
        {"entry_count", 0},
        {"entries", nlohmann::json::array()},
    };
    if (store_catalog_) {
        snapshot["store"]["entry_count"] = store_catalog_->entries().size();
        for (const auto& entry : store_catalog_->entries()) {
            snapshot["store"]["entries"].push_back({
                {"id", entry.id},
                {"display_name", entry.displayName},
                {"version", entry.version},
                {"publisher", entry.publisher},
                {"verified", entry.verified},
                {"mod_id", entry.manifest.id},
                {"tags", entry.tags},
            });
        }
    }

    if (!registry_) {
        last_render_snapshot_ = snapshot;
        return;
    }

    const auto manifests = registry_->listMods();
    const auto activeMods = registry_->listActiveMods();
    urpg::mod::ModRegistryValidator validator;
    const auto issues = validator.validate(manifests);

    snapshot["registered_mod_ids"] = nlohmann::json::array();
    snapshot["mods"] = nlohmann::json::array();
    for (const auto& manifest : manifests) {
        snapshot["registered_mod_ids"].push_back(manifest.id);
        nlohmann::json modSnapshot = {
            {"id", manifest.id},
            {"name", manifest.name},
            {"version", manifest.version},
            {"entry_point", manifest.entryPoint},
            {"dependencies", manifest.dependencies},
            {"active", std::find(activeMods.begin(), activeMods.end(), manifest.id) != activeMods.end()},
            {"sandbox_policy", nullptr},
            {"contract_failures", nlohmann::json::array()},
        };

        if (loader_) {
            if (const auto policy = loader_->getSandboxPolicy(manifest.id)) {
                modSnapshot["sandbox_policy"] = sandboxPolicyToJson(*policy);
            }
            for (const auto& failure : loader_->validateContract(manifest)) {
                modSnapshot["contract_failures"].push_back({
                    {"mod_id", failure.modId},
                    {"field", failure.field},
                    {"reason", failure.reason},
                });
            }
        }

        snapshot["mods"].push_back(std::move(modSnapshot));
    }
    snapshot["active_count"] = activeMods.size();
    snapshot["validation_issue_count"] = issues.size();
    snapshot["validation_issues"] = nlohmann::json::array();
    for (const auto& issue : issues) {
        snapshot["validation_issues"].push_back({
            {"mod_id", issue.modId},
            {"category", issueCategoryToString(issue.category)},
            {"severity", issueSeverityToString(issue.severity)},
            {"message", issue.message},
        });
    }

    try {
        const auto loadOrder = registry_->resolveLoadOrder();
        snapshot["resolved_load_order"] = loadOrder;
        snapshot["registered_count"] = manifests.size();
    } catch (const std::runtime_error& e) {
        snapshot["cycle_warning"] = e.what();
        snapshot["registered_count"] = manifests.size();
    }
    if (!snapshot["cycle_warning"].is_null()) {
        snapshot["status"] = "error";
        snapshot["empty_reason"] = "";
        snapshot["error_message"] = snapshot["cycle_warning"];
    } else if (!issues.empty()) {
        snapshot["status"] = "error";
        snapshot["empty_reason"] = "";
        snapshot["error_message"] = issues.front().message;
    } else if (!manifests.empty()) {
        snapshot["status"] = "ready";
        snapshot["empty_reason"] = "";
    }

    last_render_snapshot_ = snapshot;
}

bool ModManagerPanel::importManifest(const std::filesystem::path& manifest_path,
                                     const urpg::mod::ModSandboxPolicy& policy) {
    if (!loader_) {
        last_action_snapshot_ = {
            {"action", "import_manifest"},
            {"success", false},
            {"mod_id", ""},
            {"error", "No mod loader is bound."},
        };
        render();
        return false;
    }

    try {
        std::ifstream input(manifest_path);
        if (!input) {
            last_action_snapshot_ = {
                {"action", "import_manifest"},
                {"success", false},
                {"mod_id", ""},
                {"error", "Failed to open manifest: " + manifest_path.string()},
            };
            render();
            return false;
        }

        nlohmann::json manifestJson;
        input >> manifestJson;
        return registerManifest(parseManifestJson(manifestJson), policy);
    } catch (const std::exception& e) {
        last_action_snapshot_ = {
            {"action", "import_manifest"},
            {"success", false},
            {"mod_id", ""},
            {"error", e.what()},
        };
        render();
        return false;
    }
}

bool ModManagerPanel::registerManifest(const urpg::mod::ModManifest& manifest,
                                       const urpg::mod::ModSandboxPolicy& policy) {
    if (!loader_) {
        last_action_snapshot_ = {
            {"action", "register_manifest"},
            {"success", false},
            {"mod_id", manifest.id},
            {"error", "No mod loader is bound."},
        };
        render();
        return false;
    }

    const auto result = loader_->registerMod(manifest, policy);
    recordLoadResult("register_manifest", result);
    render();
    return result.success;
}

bool ModManagerPanel::activateMod(const std::string& id) {
    if (!loader_) {
        last_action_snapshot_ = {
            {"action", "activate"},
            {"success", false},
            {"mod_id", id},
            {"error", "No mod loader is bound."},
        };
        render();
        return false;
    }

    const auto result = loader_->activateMod(id);
    recordLoadResult("activate", result);
    render();
    return result.success;
}

bool ModManagerPanel::deactivateMod(const std::string& id) {
    if (!loader_) {
        last_action_snapshot_ = {
            {"action", "deactivate"},
            {"success", false},
            {"mod_id", id},
            {"error", "No mod loader is bound."},
        };
        render();
        return false;
    }

    const auto result = loader_->deactivateMod(id);
    recordLoadResult("deactivate", result);
    render();
    return result.success;
}

bool ModManagerPanel::reloadMod(const std::string& id) {
    if (!loader_) {
        last_action_snapshot_ = {
            {"action", "reload"},
            {"success", false},
            {"mod_id", id},
            {"error", "No mod loader is bound."},
        };
        render();
        return false;
    }

    const auto result = loader_->reloadMod(id);
    recordLoadResult("reload", result);
    render();
    return result.success;
}

bool ModManagerPanel::trackHotLoadMod(const std::string& id) {
    if (!hot_loader_) {
        last_action_snapshot_ = {
            {"action", "track_hot_load_mod"},
            {"success", false},
            {"mod_id", id},
            {"error", "No mod hot-loader is bound."},
        };
        render();
        return false;
    }

    const bool tracked = hot_loader_->trackMod(id);
    last_action_snapshot_ = {
        {"action", "track_hot_load_mod"},
        {"success", tracked},
        {"mod_id", id},
        {"error", tracked ? "" : "Mod is unknown or has no hot-load entrypoint."},
    };
    render();
    return tracked;
}

size_t ModManagerPanel::trackRegisteredHotLoadMods() {
    if (!registry_ || !hot_loader_) {
        last_action_snapshot_ = {
            {"action", "track_registered_hot_load_mods"},
            {"success", false},
            {"mod_id", ""},
            {"error", !registry_ ? "No mod registry is bound." : "No mod hot-loader is bound."},
            {"tracked_count", 0},
        };
        render();
        return 0;
    }

    size_t trackedCount = 0;
    for (const auto& manifest : registry_->listMods()) {
        if (hot_loader_->trackMod(manifest.id)) {
            ++trackedCount;
        }
    }
    last_action_snapshot_ = {
        {"action", "track_registered_hot_load_mods"},
        {"success", trackedCount > 0},
        {"mod_id", ""},
        {"error", trackedCount > 0 ? "" : "No registered mods have hot-load entrypoints."},
        {"tracked_count", trackedCount},
    };
    render();
    return trackedCount;
}

urpg::mod::ModHotLoadPollResult ModManagerPanel::pollHotLoader() {
    if (!hot_loader_) {
        last_hot_poll_result_ = {};
        last_action_snapshot_ = {
            {"action", "poll_hot_loader"},
            {"success", false},
            {"mod_id", ""},
            {"error", "No mod hot-loader is bound."},
            {"event_count", 0},
            {"any_reloaded", false},
        };
        render();
        return {};
    }

    last_hot_poll_result_ = hot_loader_->poll();
    last_action_snapshot_ = {
        {"action", "poll_hot_loader"},
        {"success", true},
        {"mod_id", ""},
        {"error", ""},
        {"event_count", last_hot_poll_result_.events.size()},
        {"any_reloaded", last_hot_poll_result_.anyReloaded},
    };
    render();
    return last_hot_poll_result_;
}

void ModManagerPanel::clearHotLoadEventLog() {
    if (hot_loader_) {
        hot_loader_->clearEventLog();
    }
    last_hot_poll_result_ = {};
    last_action_snapshot_ = {
        {"action", "clear_hot_load_events"},
        {"success", hot_loader_ != nullptr},
        {"mod_id", ""},
        {"error", hot_loader_ ? "" : "No mod hot-loader is bound."},
    };
    render();
}

bool ModManagerPanel::installStoreEntry(const std::string& entry_id, const std::filesystem::path& catalog_root) {
    if (!loader_ || !store_catalog_) {
        last_action_snapshot_ = {
            {"action", "install_store_entry"},
            {"success", false},
            {"entry_id", entry_id},
            {"mod_id", ""},
            {"error", !loader_ ? "No mod loader is bound." : "No mod store catalog is bound."},
        };
        render();
        return false;
    }

    const auto entry = store_catalog_->findEntry(entry_id);
    if (!entry.has_value()) {
        last_action_snapshot_ = {
            {"action", "install_store_entry"},
            {"success", false},
            {"entry_id", entry_id},
            {"mod_id", ""},
            {"error", "Unknown mod store entry: " + entry_id},
        };
        render();
        return false;
    }

    urpg::mod::ModStoreInstaller installer(*loader_);
    const auto result = installer.install(*entry, catalog_root);
    last_action_snapshot_ = {
        {"action", "install_store_entry"},
        {"success", result.success},
        {"entry_id", result.entryId},
        {"mod_id", result.modId},
        {"error", result.errorMessage},
        {"load_order", result.loadOrder},
    };
    render();
    return result.success;
}

void ModManagerPanel::clearLastAction() {
    last_action_snapshot_ = nlohmann::json::object();
}

nlohmann::json ModManagerPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

urpg::mod::ModManifest ModManagerPanel::parseManifestJson(const nlohmann::json& manifest_json) {
    if (!manifest_json.is_object()) {
        throw std::runtime_error("Mod manifest must be a JSON object.");
    }

    urpg::mod::ModManifest manifest;
    manifest.id = manifest_json.value("id", "");
    manifest.name = manifest_json.value("name", "");
    manifest.version = manifest_json.value("version", "");
    manifest.author = manifest_json.value("author", "");
    manifest.entryPoint = manifest_json.value("entryPoint", "");

    if (const auto it = manifest_json.find("dependencies"); it != manifest_json.end() && it->is_array()) {
        for (const auto& dependency : *it) {
            if (dependency.is_string()) {
                manifest.dependencies.push_back(dependency.get<std::string>());
            }
        }
    }

    return manifest;
}

void ModManagerPanel::recordLoadResult(const std::string& action, const urpg::mod::ModLoadResult& result) {
    last_action_snapshot_ = {
        {"action", action},
        {"success", result.success},
        {"mod_id", result.modId},
        {"error", result.errorMessage},
        {"load_order", result.loadOrder},
    };
}

} // namespace urpg::editor
