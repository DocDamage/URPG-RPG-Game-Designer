#pragma once

#include "engine/core/items/loot_affix_generator.h"

#include <nlohmann/json.hpp>

namespace urpg::editor::items {

struct LootGeneratorPanelSnapshot {
    std::string table_id;
    uint64_t seed = 0;
    std::size_t item_count = 0;
    std::size_t diagnostic_count = 0;
};

class LootGeneratorPanel {
public:
    void loadDocument(urpg::items::LootGeneratorDocument document);
    void setPreviewSeed(uint64_t seed, std::size_t count);
    void render();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::items::LootGeneratorPreview& preview() const { return preview_; }
    [[nodiscard]] const LootGeneratorPanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::items::LootGeneratorDocument document_;
    uint64_t seed_ = 0;
    std::size_t count_ = 4;
    urpg::items::LootGeneratorPreview preview_{};
    LootGeneratorPanelSnapshot snapshot_{};
};

} // namespace urpg::editor::items
