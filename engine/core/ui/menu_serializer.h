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
     *
     * For backward compatibility, this exports the first registered scene only.
     * Use SerializeGraph to export all scenes.
     */
    static nlohmann::json Serialize(const MenuSceneGraph& graph);

    /**
     * @brief Serialize all scenes in a MenuSceneGraph.
     */
    static nlohmann::json SerializeGraph(const MenuSceneGraph& graph);

    /**
     * @brief Populate a MenuSceneGraph from a multi-scene native URPG JSON format.
     */
    static bool DeserializeGraph(const nlohmann::json& j, MenuSceneGraph& graph);

    /**
     * @brief Import legacy RPG Maker (MV/MZ) menu data into URPG format.
     *
     * Handles mapping of Window_Command children and Handler bindings.
     */
    static bool ImportLegacy(const nlohmann::json& legacy_data, MenuSceneGraph& out_graph);
};

} // namespace urpg::ui
