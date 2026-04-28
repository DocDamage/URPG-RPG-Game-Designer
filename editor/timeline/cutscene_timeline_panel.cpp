#include "editor/timeline/cutscene_timeline_panel.h"

#include <utility>

namespace urpg::editor {

void CutsceneTimelinePanel::loadDocument(timeline::CutsceneTimelineDocument document) {
    document_ = std::move(document);
    refresh();
}

void CutsceneTimelinePanel::setPreviewContext(int64_t cursor_tick, uint64_t seed, std::set<std::string> localization_keys) {
    cursor_tick_ = cursor_tick;
    seed_ = seed;
    localization_keys_ = std::move(localization_keys);
    refresh();
}

void CutsceneTimelinePanel::render() {
    refresh();
}

nlohmann::json CutsceneTimelinePanel::saveProjectData() const {
    return document_.toJson();
}

void CutsceneTimelinePanel::refresh() {
    preview_ = document_.preview(cursor_tick_, seed_, localization_keys_);
    snapshot_.cutscene_id = preview_.cutscene_id;
    snapshot_.cursor_tick = cursor_tick_;
    snapshot_.active_cue_count = preview_.active_cues.size();
    snapshot_.played_command_count = preview_.played_commands.size();
    snapshot_.variable_write_count = preview_.variable_writes.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor
