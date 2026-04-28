#include "editor/monster/monster_collection_panel.h"

#include <utility>

namespace urpg::editor {

void MonsterCollectionPanel::bindDocument(urpg::monster::MonsterCollectionDocument document) {
    document_ = std::move(document);
}

void MonsterCollectionPanel::setCaptureAttempt(urpg::monster::CaptureAttempt attempt) {
    attempt_ = std::move(attempt);
}

bool MonsterCollectionPanel::capturePreviewedMonster(const std::string& instance_id) {
    return document_.capture(attempt_, instance_id);
}

void MonsterCollectionPanel::render() {
    snapshot_ = {{"panel", "monster_collection"},
                 {"document", document_.toJson()},
                 {"capture_attempt",
                  {{"species_id", attempt_.species_id},
                   {"target_hp_percent", attempt_.target_hp_percent},
                   {"ball_bonus", attempt_.ball_bonus},
                   {"seed", attempt_.seed}}},
                 {"capture_preview", urpg::monster::monsterCapturePreviewToJson(document_.previewCapture(attempt_))}};
}

nlohmann::json MonsterCollectionPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
