#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

enum class PlaytestResourceClass : uint8_t {
    Map,
    Dialogue,
    Quest,
    Database,
    Localization,
    Audio,
    Script,
    Plugin,
    Renderer,
};

enum class PlaytestReloadBehavior : uint8_t { HotReloadable, RestartRequired, Rejected };
enum class PlaytestReloadStatePolicy : uint8_t { Preserve, ResetAffected, ResetAll };
enum class PlaytestReloadAction : uint8_t { Apply, Rollback, Restart };
enum class PlaytestReloadStatus : uint8_t { Applied, RestartQueued, Rejected, FailedRecovered, FailedUnrecovered };

std::string_view playtestResourceClassName(PlaytestResourceClass resource_class);
std::string_view playtestReloadBehaviorName(PlaytestReloadBehavior behavior);

struct PlaytestReloadCapability {
    PlaytestResourceClass resource_class = PlaytestResourceClass::Map;
    PlaytestReloadBehavior behavior = PlaytestReloadBehavior::Rejected;
    bool supports_state_preservation = false;
    std::string reason;
};

struct PlaytestHotReloadRequest {
    std::string request_id;
    PlaytestResourceClass resource_class = PlaytestResourceClass::Map;
    std::string resource_id;
    std::string content;
    uint64_t expected_revision = 0;
    PlaytestReloadStatePolicy state_policy = PlaytestReloadStatePolicy::Preserve;
    bool allow_restart_fallback = true;
};

struct PlaytestReloadCommand {
    PlaytestReloadAction action = PlaytestReloadAction::Apply;
    PlaytestHotReloadRequest request;
    uint64_t previous_revision = 0;
};

struct PlaytestReloadRuntimeAck {
    bool success = false;
    bool state_changed = false;
    std::string code;
    std::string message;
};

struct PlaytestHotReloadResult {
    PlaytestReloadStatus status = PlaytestReloadStatus::Rejected;
    PlaytestReloadCapability capability;
    PlaytestReloadStatePolicy effective_state_policy = PlaytestReloadStatePolicy::Preserve;
    bool valid = false;
    bool recovery_attempted = false;
    bool recovered = false;
    bool restart_fallback = false;
    uint64_t resulting_revision = 0;
    std::string code;
    std::string message;
};

class PlaytestHotReloadCoordinator {
public:
    using Transport = std::function<PlaytestReloadRuntimeAck(const PlaytestReloadCommand&)>;

    explicit PlaytestHotReloadCoordinator(Transport transport);

    static std::vector<PlaytestReloadCapability> productCapabilityMatrix();
    bool negotiate(const nlohmann::json& runtime_capabilities, std::string* diagnostic = nullptr);
    const std::vector<PlaytestReloadCapability>& negotiatedCapabilities() const { return negotiated_; }

    PlaytestHotReloadResult preview(const PlaytestHotReloadRequest& request) const;
    PlaytestHotReloadResult execute(const PlaytestHotReloadRequest& request);
    uint64_t revision(PlaytestResourceClass resource_class, std::string_view resource_id) const;

    static constexpr std::size_t kMaxReloadBytes = 4U * 1024U * 1024U;

private:
    const PlaytestReloadCapability* capability(PlaytestResourceClass resource_class) const;
    std::string revisionKey(PlaytestResourceClass resource_class, std::string_view resource_id) const;

    Transport transport_;
    std::vector<PlaytestReloadCapability> negotiated_;
    std::map<std::string, uint64_t> revisions_;
};

} // namespace urpg::editor
