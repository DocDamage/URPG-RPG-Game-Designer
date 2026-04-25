#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace urpg::timeline {

enum class TimelineCommandKind : uint8_t {
    Message,
    Movement,
    Audio,
    Fade,
    Tint,
    Camera,
    Wait,
    EventCall,
    BattleCue,
    Unsupported
};

struct TimelineCommand {
    std::string id;
    TimelineCommandKind kind = TimelineCommandKind::Unsupported;
    int64_t tick = 0;
    int64_t duration = 0;
    std::string actor_id;
    std::string target;
    nlohmann::json payload = nlohmann::json::object();

    bool operator==(const TimelineCommand& other) const = default;
};

struct TimelineDiagnostic {
    std::string code;
    std::string message;
    std::string command_id;
};

class TimelineDocument {
public:
    void addActor(std::string actor_id);
    void addCommand(TimelineCommand command);

    const std::set<std::string>& actors() const { return actors_; }
    const std::vector<TimelineCommand>& commands() const { return commands_; }

    std::vector<TimelineCommand> sortedCommands() const;
    std::vector<TimelineDiagnostic> validate() const;
    nlohmann::json toJson() const;

    static TimelineDocument fromJson(const nlohmann::json& json);

private:
    std::set<std::string> actors_;
    std::vector<TimelineCommand> commands_;
};

std::string toString(TimelineCommandKind kind);
TimelineCommandKind timelineCommandKindFromString(const std::string& value);

} // namespace urpg::timeline
