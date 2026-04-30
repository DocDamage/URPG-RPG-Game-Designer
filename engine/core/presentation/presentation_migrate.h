#pragma once

#include "presentation_schema.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::presentation {

/**
 * @brief Bridge for migrating legacy 2D map data to First-Class Spatial schemas.
 * (ADR-008: Project Migration Strategy)
 */
class PresentationMigrationTool {
  public:
    struct LegacyMapData {
        std::string id;
        uint32_t width;
        uint32_t height;
        std::vector<int> tileData; // Legacy flat tiles
    };

    /**
     * @brief Transforms legacy project structures into the new Presentation format.
     */
    static SpatialMapOverlay MigrateMap(const LegacyMapData& legacy) {
        SpatialMapOverlay spatial;
        spatial.mapId = legacy.id;
        spatial.schemaVersion = 1;

        // 1. Initialize Elevation Grid from dimensions
        spatial.elevation.width = legacy.width;
        spatial.elevation.height = legacy.height;
        spatial.elevation.stepHeight = 0.5f;
        spatial.elevation.levels.assign(legacy.width * legacy.height, 0);

        // 2. Heuristic: Infer elevation from "Legacy Tile IDs"
        // Example: Tiles > 100 are considered "high ground" in our mock logic
        for (size_t i = 0; i < legacy.tileData.size(); ++i) {
            if (legacy.tileData[i] > 100) {
                spatial.elevation.levels[i] = 2; // +1.0m elevation
            }
        }

        // 3. Inject standard environmental defaults for migrated maps
        spatial.fog.density = 0.05f;
        spatial.postFX.bloomIntensity = 0.2f;

        return spatial;
    }

    /**
     * @brief Serialize SpatialMapOverlay to JSON for the new content pipeline.
     */
    static nlohmann::json ToJson(const SpatialMapOverlay& overlay) {
        nlohmann::json j;
        j["mapId"] = overlay.mapId;
        j["schemaVersion"] = overlay.schemaVersion;

        j["elevation"]["width"] = overlay.elevation.width;
        j["elevation"]["height"] = overlay.elevation.height;
        j["elevation"]["levels"] = overlay.elevation.levels;
        j["elevation"]["stepHeight"] = overlay.elevation.stepHeight;

        // Serialize Props
        nlohmann::json props = nlohmann::json::array();
        for (const auto& prop : overlay.props) {
            nlohmann::json p;
            if (!prop.instanceId.empty()) {
                p["instanceId"] = prop.instanceId;
            }
            p["assetId"] = prop.assetId;
            p["posX"] = prop.posX;
            p["posY"] = prop.posY;
            p["posZ"] = prop.posZ;
            p["rotY"] = prop.rotY;
            p["scale"] = prop.scale;
            props.push_back(p);
        }
        j["props"] = props;

        // Serialize Lights
        nlohmann::json lights = nlohmann::json::array();
        for (const auto& light : overlay.lights) {
            nlohmann::json l;
            l["lightId"] = light.lightId;
            l["type"] = static_cast<int>(light.type);
            l["position"] = {light.position.x, light.position.y, light.position.z};
            l["color"] = {light.color[0], light.color[1], light.color[2]};
            l["intensity"] = light.intensity;
            l["range"] = light.range;
            lights.push_back(l);
        }
        j["lights"] = lights;

        j["fog"]["density"] = overlay.fog.density;
        j["fog"]["color"] = {overlay.fog.color[0], overlay.fog.color[1], overlay.fog.color[2]};
        j["fog"]["startDist"] = overlay.fog.startDist;
        j["fog"]["endDist"] = overlay.fog.endDist;

        j["postFX"]["bloomIntensity"] = overlay.postFX.bloomIntensity;
        j["postFX"]["exposure"] = overlay.postFX.exposure;
        j["postFX"]["bloomThreshold"] = overlay.postFX.bloomThreshold;
        j["postFX"]["saturation"] = overlay.postFX.saturation;

        return j;
    }

    /**
     * @brief Deserialize SpatialMapOverlay from JSON.
     */
    static SpatialMapOverlay FromJson(const nlohmann::json& j) {
        SpatialMapOverlay overlay;
        overlay.mapId = j.value("mapId", "");
        overlay.schemaVersion = j.value("schemaVersion", 1u);

        if (j.contains("elevation")) {
            overlay.elevation.width = j["elevation"].value("width", 0u);
            overlay.elevation.height = j["elevation"].value("height", 0u);
            overlay.elevation.levels = j["elevation"].value("levels", std::vector<int8_t>());
            overlay.elevation.stepHeight = j["elevation"].value("stepHeight", 0.5f);
        }

        if (j.contains("props") && j["props"].is_array()) {
            for (const auto& p : j["props"]) {
                PropInstance instance;
                instance.instanceId = p.value("instanceId", "");
                instance.assetId = p.value("assetId", "");
                instance.posX = p.value("posX", 0.0f);
                instance.posY = p.value("posY", 0.0f);
                instance.posZ = p.value("posZ", 0.0f);
                instance.rotY = p.value("rotY", 0.0f);
                instance.scale = p.value("scale", 1.0f);
                overlay.props.push_back(instance);
            }
            EnsureStablePropInstanceIds(overlay);
        }

        if (j.contains("lights") && j["lights"].is_array()) {
            for (const auto& l : j["lights"]) {
                LightProfile light;
                light.lightId = l.value("lightId", 0u);
                light.type = static_cast<LightProfile::LightType>(l.value("type", 1));
                if (l.contains("position") && l["position"].is_array() && l["position"].size() == 3) {
                    light.position = {l["position"][0], l["position"][1], l["position"][2]};
                }
                if (l.contains("color") && l["color"].is_array() && l["color"].size() == 3) {
                    light.color[0] = l["color"][0];
                    light.color[1] = l["color"][1];
                    light.color[2] = l["color"][2];
                }
                light.intensity = l.value("intensity", 1.0f);
                light.range = l.value("range", 10.0f);
                overlay.lights.push_back(light);
            }
        }

        if (j.contains("fog")) {
            overlay.fog.density = j["fog"].value("density", 0.01f);
            if (j["fog"].contains("color") && j["fog"]["color"].is_array() && j["fog"]["color"].size() == 3) {
                overlay.fog.color[0] = j["fog"]["color"][0];
                overlay.fog.color[1] = j["fog"]["color"][1];
                overlay.fog.color[2] = j["fog"]["color"][2];
            }
            overlay.fog.startDist = j["fog"].value("startDist", 0.0f);
            overlay.fog.endDist = j["fog"].value("endDist", 100.0f);
        }

        if (j.contains("postFX")) {
            overlay.postFX.exposure = j["postFX"].value("exposure", 1.0f);
            overlay.postFX.bloomThreshold = j["postFX"].value("bloomThreshold", 1.0f);
            overlay.postFX.bloomIntensity = j["postFX"].value("bloomIntensity", 0.0f);
            overlay.postFX.saturation = j["postFX"].value("saturation", 1.0f);
        }

        return overlay;
    }
};

} // namespace urpg::presentation
