#pragma once

#include "engine/core/ui/menu_scene_graph.h"
#include <nlohmann/json.hpp>

namespace urpg::ui {

/**
 * @brief Serialization and Import logic for MenuSceneGraph.
 */
class MenuSceneSerializer {
public:
    /**
     * @brief Populate a MenuSceneGraph from the native URPG JSON format.
     */
    static bool Deserialize(const nlohmann::json& j, MenuSceneGraph& graph);

    /**
     * @brief Serialize a MenuSceneGraph to the native URPG JSON format.
     */
    static nlohmann::json Serialize(const MenuSceneGraph& graph);

    /**
     * @brief Import legacy RPG Maker (MV/MZ) menu data into URPG format.
     * 
     * Handles mapping of Window_Command children and Handler bindings.
     */
    static bool ImportLegacy(const nlohmann::json& legacy_data, MenuSceneGraph& out_graph);
};

} // namespace urpg::ui
