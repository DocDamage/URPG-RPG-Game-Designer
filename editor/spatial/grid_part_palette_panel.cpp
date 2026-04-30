#include "editor/spatial/grid_part_palette_panel.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

void GridPartPalettePanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void GridPartPalettePanel::SetCatalog(const urpg::map::GridPartCatalog* catalog) {
    catalog_ = catalog;
    if (catalog_ == nullptr || (!selected_part_id_.empty() && catalog_->find(selected_part_id_) == nullptr)) {
        selected_part_id_.clear();
    }
    captureRenderSnapshot();
}

void GridPartPalettePanel::SetSearchQuery(std::string query) {
    search_query_ = std::move(query);
    captureRenderSnapshot();
}

void GridPartPalettePanel::SetCategoryFilter(std::optional<urpg::map::GridPartCategory> category) {
    category_filter_ = category;
    captureRenderSnapshot();
}

bool GridPartPalettePanel::SelectPart(const std::string& part_id) {
    if (catalog_ == nullptr || catalog_->find(part_id) == nullptr) {
        return false;
    }

    selected_part_id_ = part_id;
    captureRenderSnapshot();
    return true;
}

bool GridPartPalettePanel::SelectPartByIndex(size_t index) {
    const auto& entries = last_render_snapshot_.entries;
    if (index >= entries.size()) {
        return false;
    }

    return SelectPart(entries[index].part_id);
}

void GridPartPalettePanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.search_query = search_query_;
    last_render_snapshot_.selected_part_id = selected_part_id_;
    if (catalog_ == nullptr) {
        return;
    }

    last_render_snapshot_.has_catalog = true;
    auto definitions = search_query_.empty() ? catalog_->allDefinitions() : catalog_->search(search_query_);
    if (category_filter_.has_value()) {
        definitions.erase(std::remove_if(definitions.begin(), definitions.end(),
                                         [&](const urpg::map::GridPartDefinition& definition) {
                                             return definition.category != *category_filter_;
                                         }),
                          definitions.end());
    }

    last_render_snapshot_.part_count = definitions.size();
    last_render_snapshot_.entries.reserve(definitions.size());
    for (const auto& definition : definitions) {
        last_render_snapshot_.entries.push_back({
            definition.part_id,
            definition.display_name,
            categoryName(definition.category),
            definition.part_id == selected_part_id_,
        });
    }
}

std::string GridPartPalettePanel::categoryName(urpg::map::GridPartCategory category) {
    switch (category) {
    case urpg::map::GridPartCategory::Tile:
        return "tile";
    case urpg::map::GridPartCategory::Wall:
        return "wall";
    case urpg::map::GridPartCategory::Platform:
        return "platform";
    case urpg::map::GridPartCategory::Hazard:
        return "hazard";
    case urpg::map::GridPartCategory::Door:
        return "door";
    case urpg::map::GridPartCategory::Npc:
        return "npc";
    case urpg::map::GridPartCategory::Enemy:
        return "enemy";
    case urpg::map::GridPartCategory::TreasureChest:
        return "treasure_chest";
    case urpg::map::GridPartCategory::SavePoint:
        return "save_point";
    case urpg::map::GridPartCategory::Trigger:
        return "trigger";
    case urpg::map::GridPartCategory::CutsceneZone:
        return "cutscene_zone";
    case urpg::map::GridPartCategory::Shop:
        return "shop";
    case urpg::map::GridPartCategory::QuestItem:
        return "quest_item";
    case urpg::map::GridPartCategory::Prop:
        return "prop";
    case urpg::map::GridPartCategory::LevelBlock:
        return "level_block";
    }
    return "prop";
}

} // namespace urpg::editor
