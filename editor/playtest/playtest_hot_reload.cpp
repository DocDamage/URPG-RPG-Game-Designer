#include "editor/playtest/playtest_hot_reload.h"

#include <algorithm>
#include <cctype>
#include <optional>

namespace urpg::editor {

namespace {

std::optional<PlaytestResourceClass> resourceClassFromName(std::string_view name) {
    for (const auto& capability : PlaytestHotReloadCoordinator::productCapabilityMatrix()) {
        if (playtestResourceClassName(capability.resource_class) == name) return capability.resource_class;
    }
    return std::nullopt;
}

std::optional<PlaytestReloadBehavior> behaviorFromName(std::string_view name) {
    if (name == "hot_reloadable") return PlaytestReloadBehavior::HotReloadable;
    if (name == "restart_required") return PlaytestReloadBehavior::RestartRequired;
    if (name == "rejected") return PlaytestReloadBehavior::Rejected;
    return std::nullopt;
}

bool safeResourceId(std::string_view id) {
    return !id.empty() && id.front() != '/' && id.find("..") == std::string_view::npos &&
           std::all_of(id.begin(), id.end(), [](unsigned char character) {
        return std::isalnum(character) || character == '.' || character == '_' || character == '-' ||
               character == ':' || character == '/';
    });
}

bool requiresJson(PlaytestResourceClass resource_class) {
    return resource_class != PlaytestResourceClass::Audio && resource_class != PlaytestResourceClass::Script &&
           resource_class != PlaytestResourceClass::Plugin && resource_class != PlaytestResourceClass::Renderer;
}

} // namespace

std::string_view playtestResourceClassName(PlaytestResourceClass resource_class) {
    switch (resource_class) {
    case PlaytestResourceClass::Map: return "map";
    case PlaytestResourceClass::Dialogue: return "dialogue";
    case PlaytestResourceClass::Quest: return "quest";
    case PlaytestResourceClass::Database: return "database";
    case PlaytestResourceClass::Localization: return "localization";
    case PlaytestResourceClass::Audio: return "audio";
    case PlaytestResourceClass::Script: return "script";
    case PlaytestResourceClass::Plugin: return "plugin";
    case PlaytestResourceClass::Renderer: return "renderer";
    }
    return "unknown";
}

std::string_view playtestReloadBehaviorName(PlaytestReloadBehavior behavior) {
    switch (behavior) {
    case PlaytestReloadBehavior::HotReloadable: return "hot_reloadable";
    case PlaytestReloadBehavior::RestartRequired: return "restart_required";
    case PlaytestReloadBehavior::Rejected: return "rejected";
    }
    return "rejected";
}

PlaytestHotReloadCoordinator::PlaytestHotReloadCoordinator(Transport transport) : transport_(std::move(transport)) {}

std::vector<PlaytestReloadCapability> PlaytestHotReloadCoordinator::productCapabilityMatrix() {
    return {
        {PlaytestResourceClass::Map, PlaytestReloadBehavior::HotReloadable, false,
         "Map reload resets affected map entities."},
        {PlaytestResourceClass::Dialogue, PlaytestReloadBehavior::HotReloadable, false,
         "Dialogue reload resets the active conversation."},
        {PlaytestResourceClass::Quest, PlaytestReloadBehavior::RestartRequired, false,
         "Quest state requires a checkpoint-safe restart."},
        {PlaytestResourceClass::Database, PlaytestReloadBehavior::RestartRequired, false,
         "Database identity and runtime caches are rebuilt on restart."},
        {PlaytestResourceClass::Localization, PlaytestReloadBehavior::HotReloadable, true,
         "Localized strings can preserve runtime state."},
        {PlaytestResourceClass::Audio, PlaytestReloadBehavior::HotReloadable, true,
         "Audio resources can preserve gameplay state."},
        {PlaytestResourceClass::Script, PlaytestReloadBehavior::RestartRequired, false,
         "Controlled scripts require a fresh runtime context."},
        {PlaytestResourceClass::Plugin, PlaytestReloadBehavior::Rejected, false,
         "Plugin reload is outside the native bounded reload contract."},
        {PlaytestResourceClass::Renderer, PlaytestReloadBehavior::Rejected, false,
         "Renderer mutation is rejected during playtest."},
    };
}

bool PlaytestHotReloadCoordinator::negotiate(const nlohmann::json& runtime_capabilities, std::string* diagnostic) {
    if (!runtime_capabilities.is_object() ||
        runtime_capabilities.value("schema", "") != "urpg.playtest_reload_capabilities.v1" ||
        !runtime_capabilities.contains("capabilities") || !runtime_capabilities["capabilities"].is_array()) {
        if (diagnostic) *diagnostic = "playtest_reload_capabilities_invalid";
        return false;
    }
    std::map<PlaytestResourceClass, PlaytestReloadCapability> runtime;
    for (const auto& value : runtime_capabilities["capabilities"]) {
        if (!value.is_object() || !value.contains("resource_class") || !value["resource_class"].is_string() ||
            !value.contains("behavior") || !value["behavior"].is_string()) {
            if (diagnostic) *diagnostic = "playtest_reload_capability_row_invalid";
            return false;
        }
        const auto resourceClass = resourceClassFromName(value["resource_class"].get<std::string>());
        const auto behavior = behaviorFromName(value["behavior"].get<std::string>());
        if (!resourceClass || !behavior || runtime.contains(*resourceClass)) {
            if (diagnostic) *diagnostic = "playtest_reload_capability_row_invalid";
            return false;
        }
        runtime.emplace(*resourceClass, PlaytestReloadCapability{*resourceClass, *behavior,
            value.value("supports_state_preservation", false), value.value("reason", "")});
    }
    negotiated_.clear();
    for (auto product : productCapabilityMatrix()) {
        const auto found = runtime.find(product.resource_class);
        if (found == runtime.end()) {
            product.behavior = PlaytestReloadBehavior::Rejected;
            product.supports_state_preservation = false;
            product.reason = "Runtime did not advertise this resource class.";
        } else if (product.behavior == PlaytestReloadBehavior::Rejected ||
                   found->second.behavior == PlaytestReloadBehavior::Rejected) {
            product.behavior = PlaytestReloadBehavior::Rejected;
            product.supports_state_preservation = false;
        } else if (product.behavior == PlaytestReloadBehavior::RestartRequired ||
                   found->second.behavior == PlaytestReloadBehavior::RestartRequired) {
            product.behavior = PlaytestReloadBehavior::RestartRequired;
            product.supports_state_preservation = false;
        } else {
            product.supports_state_preservation &= found->second.supports_state_preservation;
        }
        negotiated_.push_back(std::move(product));
    }
    return true;
}

PlaytestHotReloadResult PlaytestHotReloadCoordinator::preview(const PlaytestHotReloadRequest& request) const {
    PlaytestHotReloadResult result;
    result.effective_state_policy = request.state_policy;
    const auto* selected = capability(request.resource_class);
    if (selected == nullptr) {
        result.code = "playtest_reload_not_negotiated";
        result.message = "Reload capabilities must be negotiated with the runtime first.";
        return result;
    }
    result.capability = *selected;
    result.resulting_revision = revision(request.resource_class, request.resource_id);
    if (request.request_id.empty() || !safeResourceId(request.resource_id) || request.content.empty() ||
        request.content.size() > kMaxReloadBytes) {
        result.code = request.content.size() > kMaxReloadBytes ? "playtest_reload_payload_too_large" :
                                                               "playtest_reload_request_invalid";
        result.message = "Reload requires stable identity and a bounded non-empty payload.";
        return result;
    }
    if (requiresJson(request.resource_class) && nlohmann::json::parse(request.content, nullptr, false).is_discarded()) {
        result.code = "playtest_reload_payload_invalid";
        result.message = "Structured reload payload is invalid JSON.";
        return result;
    }
    if (request.expected_revision != result.resulting_revision) {
        result.code = "playtest_reload_revision_mismatch";
        result.message = "Resource changed after the reload request was prepared.";
        return result;
    }
    if (request.state_policy == PlaytestReloadStatePolicy::Preserve && !selected->supports_state_preservation) {
        result.code = "playtest_reload_preserve_unsupported";
        result.message = "This resource class requires an explicit reset policy.";
        return result;
    }
    if (selected->behavior == PlaytestReloadBehavior::Rejected) {
        result.code = "playtest_reload_resource_rejected";
        result.message = selected->reason;
        return result;
    }
    result.valid = true;
    result.code = selected->behavior == PlaytestReloadBehavior::HotReloadable ?
        "playtest_reload_ready" : "playtest_restart_required";
    result.message = selected->reason;
    return result;
}

PlaytestHotReloadResult PlaytestHotReloadCoordinator::execute(const PlaytestHotReloadRequest& request) {
    auto result = preview(request);
    if (!result.valid) return result;
    const auto runRestart = [&]() {
        if (!request.allow_restart_fallback || !transport_) return false;
        const auto ack = transport_({PlaytestReloadAction::Restart, request, request.expected_revision});
        if (!ack.success) {
            result.code = ack.code.empty() ? "playtest_restart_failed" : ack.code;
            result.message = ack.message;
            return false;
        }
        result.restart_fallback = true;
        result.status = PlaytestReloadStatus::RestartQueued;
        result.code = "playtest_restart_queued";
        result.resulting_revision = request.expected_revision + 1;
        revisions_[revisionKey(request.resource_class, request.resource_id)] = result.resulting_revision;
        return true;
    };
    if (result.capability.behavior == PlaytestReloadBehavior::RestartRequired) {
        if (!runRestart()) {
            result.valid = false;
            result.status = PlaytestReloadStatus::Rejected;
            if (result.code == "playtest_restart_required") result.code = "playtest_restart_fallback_disabled";
        }
        return result;
    }
    if (!transport_) {
        result.valid = false;
        result.code = "playtest_reload_transport_missing";
        return result;
    }
    const auto ack = transport_({PlaytestReloadAction::Apply, request, request.expected_revision});
    if (ack.success) {
        result.status = PlaytestReloadStatus::Applied;
        result.code = "playtest_reload_applied";
        result.message = ack.message;
        result.resulting_revision = request.expected_revision + 1;
        revisions_[revisionKey(request.resource_class, request.resource_id)] = result.resulting_revision;
        return result;
    }
    if (ack.state_changed) {
        result.recovery_attempted = true;
        const auto rollback = transport_({PlaytestReloadAction::Rollback, request, request.expected_revision});
        result.recovered = rollback.success;
        if (!rollback.success) {
            result.status = PlaytestReloadStatus::FailedUnrecovered;
            result.code = rollback.code.empty() ? "playtest_reload_rollback_failed" : rollback.code;
            result.message = rollback.message;
            return result;
        }
    } else {
        result.recovered = true;
    }
    if (runRestart()) return result;
    result.status = result.recovered ? PlaytestReloadStatus::FailedRecovered :
                                      PlaytestReloadStatus::FailedUnrecovered;
    result.code = result.recovered ? "playtest_reload_failed_recovered" : "playtest_reload_failed_unrecovered";
    result.message = ack.message;
    return result;
}

uint64_t PlaytestHotReloadCoordinator::revision(PlaytestResourceClass resource_class,
                                                std::string_view resource_id) const {
    const auto found = revisions_.find(revisionKey(resource_class, resource_id));
    return found == revisions_.end() ? 0 : found->second;
}

const PlaytestReloadCapability* PlaytestHotReloadCoordinator::capability(PlaytestResourceClass resource_class) const {
    const auto found = std::find_if(negotiated_.begin(), negotiated_.end(), [resource_class](const auto& value) {
        return value.resource_class == resource_class;
    });
    return found == negotiated_.end() ? nullptr : &*found;
}

std::string PlaytestHotReloadCoordinator::revisionKey(PlaytestResourceClass resource_class,
                                                      std::string_view resource_id) const {
    return std::string(playtestResourceClassName(resource_class)) + ":" + std::string(resource_id);
}

} // namespace urpg::editor
