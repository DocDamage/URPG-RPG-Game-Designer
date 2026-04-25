#include "engine/core/timeline/timeline_player.h"

namespace urpg::timeline {

std::vector<PlayedTimelineCommand> TimelinePlayer::play(const TimelineDocument& document, uint64_t seed) {
    std::vector<PlayedTimelineCommand> played;
    for (const auto& command : document.sortedCommands()) {
        played.push_back(PlayedTimelineCommand{
            command.tick,
            seed,
            command.id,
            command.kind,
            command.actor_id,
            command.target,
        });
    }
    return played;
}

} // namespace urpg::timeline
