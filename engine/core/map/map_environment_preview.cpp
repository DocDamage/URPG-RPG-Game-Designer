#include "engine/core/map/map_environment_preview.h"

#include <algorithm>
#include <set>
#include <utility>

namespace urpg::map {
namespace {

presentation::LightProfile lightForWeather(const std::string& weather, const std::string& lighting,
                                            uint32_t light_id, int32_t x, int32_t y) {
    presentation::LightProfile light;
    light.lightId = light_id;
    light.type = presentation::LightProfile::LightType::Directional;
    light.position = {static_cast<float>(x), 8.0f, static_cast<float>(y)};
    light.range = 24.0f;

    if (lighting == "night") {
        light.intensity = 0.55f;
        light.color[0] = 0.62f;
        light.color[1] = 0.70f;
        light.color[2] = 1.0f;
    } else if (weather == "rain") {
        light.intensity = 0.82f;
        light.color[0] = 0.72f;
        light.color[1] = 0.80f;
        light.color[2] = 0.95f;
    } else if (weather == "snow") {
        light.intensity = 1.05f;
        light.color[0] = 0.90f;
        light.color[1] = 0.94f;
        light.color[2] = 1.0f;
    } else if (weather == "storm") {
        light.intensity = 0.70f;
        light.color[0] = 0.58f;
        light.color[1] = 0.63f;
        light.color[2] = 0.82f;
    } else {
        light.intensity = 1.0f;
    }

    return light;
}

presentation::FogProfile fogForWeather(const std::string& weather) {
    presentation::FogProfile fog;
    fog.startDist = 0.0f;
    fog.endDist = 90.0f;

    if (weather == "rain") {
        fog.density = 0.035f;
        fog.color[0] = 0.36f;
        fog.color[1] = 0.43f;
        fog.color[2] = 0.55f;
    } else if (weather == "snow") {
        fog.density = 0.026f;
        fog.color[0] = 0.82f;
        fog.color[1] = 0.88f;
        fog.color[2] = 0.96f;
    } else if (weather == "storm") {
        fog.density = 0.050f;
        fog.color[0] = 0.22f;
        fog.color[1] = 0.25f;
        fog.color[2] = 0.34f;
        fog.endDist = 55.0f;
    } else if (weather == "fog") {
        fog.density = 0.060f;
        fog.color[0] = 0.62f;
        fog.color[1] = 0.64f;
        fog.color[2] = 0.66f;
        fog.endDist = 45.0f;
    } else {
        fog.density = 0.006f;
        fog.color[0] = 0.68f;
        fog.color[1] = 0.76f;
        fog.color[2] = 0.88f;
        fog.endDist = 140.0f;
    }

    return fog;
}

presentation::PostFXProfile postFxForWeather(const std::string& weather, const std::string& lighting) {
    presentation::PostFXProfile fx;
    fx.exposure = lighting == "night" ? 0.82f : 1.0f;
    fx.bloomThreshold = 1.0f;
    fx.bloomIntensity = lighting == "night" ? 0.20f : 0.05f;
    fx.saturation = 1.0f;

    if (weather == "rain") {
        fx.saturation = 0.82f;
        fx.bloomIntensity = 0.12f;
    } else if (weather == "snow") {
        fx.exposure = 1.08f;
        fx.saturation = 0.78f;
        fx.bloomIntensity = 0.18f;
    } else if (weather == "storm") {
        fx.exposure = 0.72f;
        fx.saturation = 0.70f;
        fx.bloomIntensity = 0.24f;
    } else if (weather == "fog") {
        fx.saturation = 0.72f;
    }

    return fx;
}

bool validWeather(const std::string& weather) {
    return weather == "clear" || weather == "rain" || weather == "snow" || weather == "storm" || weather == "fog";
}

nlohmann::json lightToJson(const presentation::LightProfile& light) {
    return {
        {"id", light.lightId},
        {"intensity", light.intensity},
        {"range", light.range},
        {"position", {light.position.x, light.position.y, light.position.z}},
        {"color", {light.color[0], light.color[1], light.color[2]}},
    };
}

nlohmann::json fogToJson(const presentation::FogProfile& fog) {
    return {
        {"density", fog.density},
        {"start_distance", fog.startDist},
        {"end_distance", fog.endDist},
        {"color", {fog.color[0], fog.color[1], fog.color[2]}},
    };
}

nlohmann::json postFxToJson(const presentation::PostFXProfile& fx) {
    return {
        {"exposure", fx.exposure},
        {"bloom_threshold", fx.bloomThreshold},
        {"bloom_intensity", fx.bloomIntensity},
        {"saturation", fx.saturation},
    };
}

float arrayFloat(const nlohmann::json& json, const char* key, size_t index, float fallback) {
    if (const auto it = json.find(key); it != json.end() && it->is_array() && it->size() > index) {
        return (*it)[index].get<float>();
    }
    return fallback;
}

presentation::LightProfile lightFromJson(const nlohmann::json& json, const presentation::LightProfile& fallback) {
    presentation::LightProfile light = fallback;
    if (!json.is_object()) {
        return light;
    }
    light.lightId = json.value("id", light.lightId);
    light.intensity = json.value("intensity", light.intensity);
    light.range = json.value("range", light.range);
    light.position = {
        arrayFloat(json, "position", 0, light.position.x),
        arrayFloat(json, "position", 1, light.position.y),
        arrayFloat(json, "position", 2, light.position.z),
    };
    light.color[0] = arrayFloat(json, "color", 0, light.color[0]);
    light.color[1] = arrayFloat(json, "color", 1, light.color[1]);
    light.color[2] = arrayFloat(json, "color", 2, light.color[2]);
    return light;
}

presentation::FogProfile fogFromJson(const nlohmann::json& json, const presentation::FogProfile& fallback) {
    presentation::FogProfile fog = fallback;
    if (!json.is_object()) {
        return fog;
    }
    fog.density = json.value("density", fog.density);
    fog.startDist = json.value("start_distance", fog.startDist);
    fog.endDist = json.value("end_distance", fog.endDist);
    fog.color[0] = arrayFloat(json, "color", 0, fog.color[0]);
    fog.color[1] = arrayFloat(json, "color", 1, fog.color[1]);
    fog.color[2] = arrayFloat(json, "color", 2, fog.color[2]);
    return fog;
}

presentation::PostFXProfile postFxFromJson(const nlohmann::json& json,
                                            const presentation::PostFXProfile& fallback) {
    presentation::PostFXProfile fx = fallback;
    if (!json.is_object()) {
        return fx;
    }
    fx.exposure = json.value("exposure", fx.exposure);
    fx.bloomThreshold = json.value("bloom_threshold", fx.bloomThreshold);
    fx.bloomIntensity = json.value("bloom_intensity", fx.bloomIntensity);
    fx.saturation = json.value("saturation", fx.saturation);
    return fx;
}

MapEnvironmentRegion regionFromJson(const nlohmann::json& json, uint32_t light_id) {
    MapEnvironmentRegion region;
    if (!json.is_object()) {
        return region;
    }

    region.id = json.value("id", "");
    region.x = json.value("x", 0);
    region.y = json.value("y", 0);
    region.width = json.value("width", 1);
    region.height = json.value("height", 1);
    region.weather = json.value("weather", "clear");
    region.lighting = json.value("lighting", "day");
    region.light = lightFromJson(json.value("light", nlohmann::json::object()),
                                 lightForWeather(region.weather, region.lighting, light_id, region.x, region.y));
    region.fog = fogFromJson(json.value("fog", nlohmann::json::object()), fogForWeather(region.weather));
    region.post_fx = postFxFromJson(json.value("post_fx", nlohmann::json::object()),
                                    postFxForWeather(region.weather, region.lighting));
    return region;
}

} // namespace

bool MapEnvironmentRegion::contains(int32_t tile_x, int32_t tile_y) const {
    return tile_x >= x && tile_y >= y && tile_x < x + width && tile_y < y + height;
}

std::vector<MapDiagnostic> MapEnvironmentPreviewDocument::validate() const {
    std::vector<MapDiagnostic> diagnostics;
    if (map_id.empty()) {
        diagnostics.push_back({"missing_map_id", "Map environment preview requires a map id.", -1, -1, ""});
    }
    if (width <= 0 || height <= 0) {
        diagnostics.push_back({"invalid_map_size", "Map environment preview requires positive dimensions.", -1, -1,
                               map_id});
    }
    if (!validWeather(base_weather)) {
        diagnostics.push_back({"unknown_weather", "Base weather is not supported by the runtime preview.", -1, -1,
                               base_weather});
    }

    std::set<std::string> ids;
    for (const auto& region : regions) {
        if (region.id.empty()) {
            diagnostics.push_back({"missing_region_id", "Environment region requires an id.", region.x, region.y, ""});
        } else if (!ids.insert(region.id).second) {
            diagnostics.push_back({"duplicate_region_id", "Environment region id must be unique.", region.x, region.y,
                                   region.id});
        }
        if (region.width <= 0 || region.height <= 0) {
            diagnostics.push_back({"invalid_region_size", "Environment region requires positive dimensions.", region.x,
                                   region.y, region.id});
        }
        if (region.x < 0 || region.y < 0 || region.x + region.width > width || region.y + region.height > height) {
            diagnostics.push_back({"region_out_of_bounds", "Environment region extends outside the map.", region.x,
                                   region.y, region.id});
        }
        if (!validWeather(region.weather)) {
            diagnostics.push_back({"unknown_weather", "Environment region weather is not supported.", region.x,
                                   region.y, region.weather});
        }
        if (region.light.intensity < 0.0f || region.light.range <= 0.0f) {
            diagnostics.push_back({"invalid_light_profile", "Environment region light profile cannot render.", region.x,
                                   region.y, region.id});
        }
        if (region.fog.density < 0.0f || region.fog.endDist <= region.fog.startDist) {
            diagnostics.push_back({"invalid_fog_profile", "Environment region fog profile cannot render.", region.x,
                                   region.y, region.id});
        }
    }

    for (size_t left = 0; left < regions.size(); ++left) {
        for (size_t right = left + 1; right < regions.size(); ++right) {
            const auto& a = regions[left];
            const auto& b = regions[right];
            const bool overlap_x = a.x < b.x + b.width && b.x < a.x + a.width;
            const bool overlap_y = a.y < b.y + b.height && b.y < a.y + a.height;
            if (overlap_x && overlap_y && a.weather != b.weather) {
                diagnostics.push_back({"region_weather_overlap", "Overlapping regions use conflicting weather.", 
                                       std::max(a.x, b.x), std::max(a.y, b.y), a.id + ":" + b.id});
            }
        }
    }

    return diagnostics;
}

const MapEnvironmentRegion* MapEnvironmentPreviewDocument::regionAt(int32_t tile_x, int32_t tile_y) const {
    for (auto it = regions.rbegin(); it != regions.rend(); ++it) {
        if (it->contains(tile_x, tile_y)) {
            return &(*it);
        }
    }
    return nullptr;
}

presentation::PresentationFrameIntent MapEnvironmentPreviewDocument::buildRuntimePreview(int32_t tile_x,
                                                                                         int32_t tile_y) const {
    presentation::PresentationFrameIntent intent;
    intent.activeMode = presentation::PresentationMode::Spatial;
    intent.activeTier = presentation::CapabilityTier::Tier2_Enhanced;
    intent.AddLight(base_light);
    intent.AddFog(base_fog, 1.0f);
    intent.AddPostFX(base_post_fx, 1.0f);

    if (const auto* region = regionAt(tile_x, tile_y)) {
        intent.AddLight(region->light);
        intent.AddFog(region->fog, 1.0f);
        intent.AddPostFX(region->post_fx, 1.0f);
    }

    presentation::PresentationRuntime::ResolveEnvironmentCommands(intent);
    return intent;
}

nlohmann::json MapEnvironmentPreviewDocument::toJson() const {
    nlohmann::json json;
    json["schema"] = "urpg.map_environment_preview.v1";
    json["map_id"] = map_id;
    json["width"] = width;
    json["height"] = height;
    json["base_weather"] = base_weather;
    json["base_light"] = lightToJson(base_light);
    json["base_fog"] = fogToJson(base_fog);
    json["base_post_fx"] = postFxToJson(base_post_fx);
    json["regions"] = nlohmann::json::array();
    for (const auto& region : regions) {
        json["regions"].push_back({
            {"id", region.id},
            {"x", region.x},
            {"y", region.y},
            {"width", region.width},
            {"height", region.height},
            {"weather", region.weather},
            {"lighting", region.lighting},
            {"light", lightToJson(region.light)},
            {"fog", fogToJson(region.fog)},
            {"post_fx", postFxToJson(region.post_fx)},
        });
    }
    return json;
}

MapEnvironmentPreviewDocument MapEnvironmentPreviewDocument::fromJson(const nlohmann::json& json) {
    MapEnvironmentPreviewDocument document;
    if (!json.is_object()) {
        return document;
    }

    document.map_id = json.value("map_id", "");
    document.width = json.value("width", 0);
    document.height = json.value("height", 0);
    document.base_weather = json.value("base_weather", "clear");
    document.base_light = lightFromJson(json.value("base_light", nlohmann::json::object()),
                                        lightForWeather(document.base_weather, "day", 1, 0, 0));
    document.base_fog = fogFromJson(json.value("base_fog", nlohmann::json::object()),
                                    fogForWeather(document.base_weather));
    document.base_post_fx = postFxFromJson(json.value("base_post_fx", nlohmann::json::object()),
                                           postFxForWeather(document.base_weather, "day"));

    uint32_t light_id = 10;
    for (const auto& row : json.value("regions", nlohmann::json::array())) {
        if (row.is_object()) {
            document.regions.push_back(regionFromJson(row, light_id++));
        }
    }
    return document;
}

MapEnvironmentPreviewDocument MapEnvironmentPreviewDocument::fromRegionRules(std::string map_id, int32_t width,
                                                                             int32_t height,
                                                                             const std::vector<MapRegionRule>& rules) {
    MapEnvironmentPreviewDocument document;
    document.map_id = std::move(map_id);
    document.width = width;
    document.height = height;
    document.base_weather = "clear";
    document.base_light = lightForWeather("clear", "day", 1, 0, 0);
    document.base_fog = fogForWeather("clear");
    document.base_post_fx = postFxForWeather("clear", "day");

    uint32_t light_id = 10;
    for (const auto& rule : rules) {
        MapEnvironmentRegion region;
        region.id = rule.id;
        region.x = rule.x;
        region.y = rule.y;
        region.width = rule.width;
        region.height = rule.height;
        region.weather = rule.weather.empty() ? document.base_weather : rule.weather;
        region.lighting = rule.hazard == "dark" ? "night" : "day";
        region.light = lightForWeather(region.weather, region.lighting, light_id++, region.x, region.y);
        region.fog = fogForWeather(region.weather);
        region.post_fx = postFxForWeather(region.weather, region.lighting);
        document.regions.push_back(region);
    }

    return document;
}

MapEnvironmentPreviewResult PreviewMapEnvironment(const MapEnvironmentPreviewDocument& document, int32_t tile_x,
                                                  int32_t tile_y) {
    MapEnvironmentPreviewResult result;
    result.region = document.regionAt(tile_x, tile_y);
    result.runtime_intent = document.buildRuntimePreview(tile_x, tile_y);
    result.diagnostics = document.validate();
    if (tile_x < 0 || tile_y < 0 || tile_x >= document.width || tile_y >= document.height) {
        result.diagnostics.push_back({"preview_tile_out_of_bounds", "Selected preview tile is outside the map.", tile_x,
                                      tile_y, document.map_id});
    }
    return result;
}

} // namespace urpg::map
