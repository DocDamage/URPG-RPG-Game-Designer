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

nlohmann::json sandboxPolicyToJson(const urpg::mod::ModSandboxPolicy& policy) {
    return {
        {"file_system_read", boolPolicyToString(policy.allowFileSystemRead)},
        {"file_system_write", boolPolicyToString(policy.allowFileSystemWrite)},
        {"network_access", boolPolicyToString(policy.allowNetworkAccess)},
        {"native_interop", boolPolicyToString(policy.allowNativeInterop)},
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

void ModManagerPanel::render() {
    nlohmann::json snapshot;
    snapshot["registered_count"] = 0;
    snapshot["active_count"] = 0;
    snapshot["resolved_load_order"] = nlohmann::json::array();
    snapshot["cycle_warning"] = nullptr;
    snapshot["registry_bound"] = registry_ != nullptr;
    snapshot["loader_bound"] = loader_ != nullptr;
    const bool actionsEnabled = registry_ != nullptr && loader_ != nullptr;
    snapshot["status_messages"] = nlohmann::json::array();
    if (registry_ == nullptr) {
        snapshot["status_messages"].push_back("No mod registry is bound.");
    }
    if (loader_ == nullptr) {
        snapshot["status_messages"].push_back("No mod loader is bound; mod actions are disabled.");
    }
    snapshot["actions"] = {
        {"import_manifest", actionsEnabled}, {"register_manifest", actionsEnabled},
        {"activate", actionsEnabled},        {"deactivate", actionsEnabled},
        {"reload", actionsEnabled},
    };
    snapshot["last_action"] = last_action_snapshot_.is_null() ? nlohmann::json::object() : last_action_snapshot_;

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
