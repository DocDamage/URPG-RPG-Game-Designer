#pragma once

#include "engine/core/monster/monster_collection.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class MonsterCollectionPanel {
public:
    void bindDocument(urpg::monster::MonsterCollectionDocument document);
    void setCaptureAttempt(urpg::monster::CaptureAttempt attempt);
    bool capturePreviewedMonster(const std::string& instance_id);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::monster::MonsterCollectionDocument document_;
    urpg::monster::CaptureAttempt attempt_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
