#include "engine/core/events/event_macro_recorder.h"

#include <algorithm>
#include <tuple>
#include <utility>

namespace urpg::events {

namespace {

EventCommandKind eventKindForAction(const std::string& action) {
    if (action == "message") {
        return EventCommandKind::Message;
    }
    if (action == "move") {
        return EventCommandKind::Transfer;
    }
    if (action == "audio") {
        return EventCommandKind::Sound;
    }
    if (action == "battle") {
        return EventCommandKind::Battle;
    }
    return EventCommandKind::Unsupported;
}

timeline::TimelineCommandKind timelineKindForAction(const std::string& action) {
    if (action == "message") {
        return timeline::TimelineCommandKind::Message;
    }
    if (action == "move") {
        return timeline::TimelineCommandKind::Movement;
    }
    if (action == "audio") {
        return timeline::TimelineCommandKind::Audio;
    }
    if (action == "battle") {
        return timeline::TimelineCommandKind::BattleCue;
    }
    return timeline::TimelineCommandKind::Unsupported;
}

std::string commandIdFor(const std::string& action, int64_t tick) {
    return action + "_" + std::to_string(tick);
}

} // namespace

void EventMacroRecorder::record(EventMacroAction action) {
    actions_.push_back(std::move(action));
}

EventMacroDraft EventMacroRecorder::finishDraft(const std::string& event_id) const {
    auto actions = actions_;
    std::stable_sort(actions.begin(), actions.end(), [](const auto& lhs, const auto& rhs) {
        return std::tie(lhs.tick, lhs.action, lhs.target) < std::tie(rhs.tick, rhs.action, rhs.target);
    });

    EventMacroDraft draft;
    draft.event_id = event_id;

    for (const auto& action : actions) {
        const auto command_id = commandIdFor(action.action, action.tick);
        const auto event_kind = eventKindForAction(action.action);
        nlohmann::json fallback = nlohmann::json::object();
        if (event_kind == EventCommandKind::Unsupported) {
            fallback = {{"action", action.action}, {"target", action.target}, {"value", action.value}};
        }

        draft.event_commands.push_back(EventCommand{
            command_id,
            event_kind,
            action.target,
            action.value,
            0,
            {},
            {},
            nlohmann::json::object(),
            fallback,
        });

        if (!action.target.empty()) {
            draft.timeline.addActor(action.target);
        }
        draft.timeline.addCommand(timeline::TimelineCommand{
            command_id,
            timelineKindForAction(action.action),
            action.tick,
            action.action == "wait" ? 1 : 0,
            action.action == "move" ? action.target : std::string{},
            action.value,
            event_kind == EventCommandKind::Unsupported ? fallback : nlohmann::json::object(),
        });
    }

    return draft;
}

} // namespace urpg::events
