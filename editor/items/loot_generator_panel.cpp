#include "editor/items/loot_generator_panel.h"

#include <utility>

namespace urpg::editor::items {

void LootGeneratorPanel::loadDocument(urpg::items::LootGeneratorDocument document) {
    document_ = std::move(document);
    refresh();
}

void LootGeneratorPanel::setPreviewSeed(uint64_t seed, std::size_t count) {
    seed_ = seed;
    count_ = count;
    refresh();
}

void LootGeneratorPanel::render() {
    refresh();
}

nlohmann::json LootGeneratorPanel::saveProjectData() const {
    return document_.toJson();
}

void LootGeneratorPanel::refresh() {
    preview_ = document_.preview(seed_, count_);
    snapshot_.table_id = preview_.table_id;
    snapshot_.seed = seed_;
    snapshot_.item_count = preview_.items.size();
    snapshot_.diagnostic_count = preview_.diagnostics.size();
}

} // namespace urpg::editor::items
