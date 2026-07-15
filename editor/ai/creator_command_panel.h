#pragma once

#include "engine/core/ai/creator_command_planner.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::editor {

class SpatialAuthoringWorkspace;

class CreatorCommandPanel {
public:
    struct TilePaletteBinding {
        int32_t planned_tile_id = 0;
        std::string planned_layer_id;
        std::string layer_id;
        std::string tileset_id;
        std::string tile_id;
    };

    struct PropAssetBinding {
        std::string planned_asset_id;
        std::string asset_id;
        std::string project_path;
    };

    struct EventLayerBinding {
        std::string planned_layer_id;
        std::string layer_id;
    };

    void setRequest(urpg::ai::CreatorCommandRequest request);
    void setTransportConfig(urpg::ai::CreatorProviderTransportConfig transportConfig);
    // Creator plans do not own a project JSON copy. Bind the active native Map
    // owner and resolve its numeric planning IDs to reviewed palette entries.
    void setMapWorkspace(SpatialAuthoringWorkspace* workspace);
    void setTilePaletteBindings(std::vector<TilePaletteBinding> bindings);
    void setPropAssetBindings(std::vector<PropAssetBinding> bindings);
    void setEventLayerBindings(std::vector<EventLayerBinding> bindings);
    void render();
    bool applyCurrentPlan();
    const nlohmann::json& lastRenderSnapshot() const;

private:
    urpg::ai::CreatorCommandRequest request_;
    urpg::ai::CreatorProviderTransportConfig transport_config_;
    SpatialAuthoringWorkspace* map_workspace_ = nullptr;
    std::vector<TilePaletteBinding> tile_palette_bindings_;
    std::vector<PropAssetBinding> prop_asset_bindings_;
    std::vector<EventLayerBinding> event_layer_bindings_;
    std::string reviewed_document_revision_;
    urpg::ai::CreatorCommandPlan current_plan_;
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
