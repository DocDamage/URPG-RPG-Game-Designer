#pragma once

#include "engine/core/timeline/timeline_document.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::timeline {

struct PlayedTimelineCommand {
    int64_t tick = 0;
    uint64_t seed = 0;
    std::string command_id;
    TimelineCommandKind kind = TimelineCommandKind::Unsupported;
    std::string actor_id;
    std::string target;

    bool operator==(const PlayedTimelineCommand& other) const = default;
};

class TimelinePlayer {
public:
    static std::vector<PlayedTimelineCommand> play(const TimelineDocument& document, uint64_t seed);
};

} // namespace urpg::timeline
