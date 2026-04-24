#include "runtimes/compat_js/data_manager_support.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <utility>

namespace urpg::compat {
namespace {
using json = nlohmann::json;
} // namespace
Value jsonToValue(const json& j) {
    if (j.is_null()) {
        return Value::Nil();
    } else if (j.is_boolean()) {
        Value v; v.v = j.get<bool>(); return v;
    } else if (j.is_number_integer()) {
        return Value::Int(j.get<int64_t>());
    } else if (j.is_number_float()) {
        Value v; v.v = j.get<double>(); return v;
    } else if (j.is_string()) {
        Value v; v.v = j.get<std::string>(); return v;
    } else if (j.is_array()) {
        Array arr;
        arr.reserve(j.size());
        for (const auto& elem : j) {
            arr.push_back(jsonToValue(elem));
        }
        return Value::Arr(std::move(arr));
    } else if (j.is_object()) {
        Object obj;
        for (auto it = j.begin(); it != j.end(); ++it) {
            obj[it.key()] = jsonToValue(it.value());
        }
        return Value::Obj(std::move(obj));
    }
    return Value::Nil();
}

std::optional<json> loadJsonFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    try {
        json j;
        file >> j;
        return j;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> loadJsonArrayFile(const std::string& filename) {
    if (DataManager::getDataDirectory().empty()) {
        return std::nullopt;
    }
    auto j = loadJsonFile(DataManager::getDataDirectory() + filename);
    if (!j || !j->is_array()) {
        return std::nullopt;
    }
    return j;
}

std::string buildMapFilename(int32_t mapId) {
    std::ostringstream filename;
    filename << "Map" << std::setw(3) << std::setfill('0') << mapId << ".json";
    return filename.str();
}

std::vector<std::vector<int32_t>> convertMapLayers(const json& data, int32_t width, int32_t height) {
    const int32_t cellCount = std::max(0, width * height);
    if (!data.is_array() || cellCount <= 0) {
        return {};
    }

    const size_t totalEntries = data.size();
    const size_t layerCount = static_cast<size_t>(std::max<int32_t>(1, static_cast<int32_t>(totalEntries / std::max(1, cellCount))));
    std::vector<std::vector<int32_t>> layers(layerCount, std::vector<int32_t>(static_cast<size_t>(cellCount), 0));

    for (size_t index = 0; index < totalEntries; ++index) {
        const size_t layer = index / static_cast<size_t>(cellCount);
        const size_t offset = index % static_cast<size_t>(cellCount);
        if (layer >= layers.size()) {
            break;
        }
        if (data[index].is_number_integer()) {
            layers[layer][offset] = data[index].get<int32_t>();
        }
    }

    return layers;
}

Object mapInfoToObject(const MapInfo& info) {
    Object obj;
    obj["id"] = Value::Int(info.id);
    obj["name"].v = info.name;
    obj["parentId"] = Value::Int(info.parentId);
    obj["order"] = Value::Int(info.order);
    obj["expanded"].v = info.expanded;
    return obj;
}

Object tilesetToObject(const TilesetData& tileset) {
    Object obj;
    obj["id"] = Value::Int(tileset.id);
    obj["name"].v = tileset.name;
    obj["mode"] = Value::Int(tileset.mode);

    Array tilesetNames;
    tilesetNames.reserve(tileset.tilesetNames.size());
    for (const auto& name : tileset.tilesetNames) {
        Value value;
        value.v = name;
        tilesetNames.push_back(std::move(value));
    }
    obj["tilesetNames"] = Value::Arr(std::move(tilesetNames));

    Array flags;
    flags.reserve(tileset.flags.size());
    for (uint32_t flag : tileset.flags) {
        flags.push_back(Value::Int(static_cast<int64_t>(flag)));
    }
    obj["flags"] = Value::Arr(std::move(flags));
    return obj;
}

void hydrateActorParamsFromClasses(std::vector<ActorData>& actors, const std::vector<ClassData>& classes) {
    for (auto& actor : actors) {
        if (!actor.params.empty() || actor.classId <= 0) {
            continue;
        }
        if (static_cast<size_t>(actor.classId) > classes.size()) {
            continue;
        }

        const auto& cls = classes[static_cast<size_t>(actor.classId - 1)];
        if (!cls.params.empty()) {
            actor.params = cls.params;
        }
    }
}

void seedActorResourceState(std::vector<ActorData>& actors) {
    const auto readParamAtLevel = [](const ActorData& actor, int32_t paramId, int32_t fallback) -> int32_t {
        if (paramId < 0 || static_cast<size_t>(paramId) >= actor.params.size()) {
            return fallback;
        }
        const auto& row = actor.params[static_cast<size_t>(paramId)];
        if (row.empty()) {
            return fallback;
        }

        const int32_t levelIndex = std::clamp(actor.level, 0, static_cast<int32_t>(row.size() - 1));
        return std::max(1, row[static_cast<size_t>(levelIndex)]);
    };

    for (auto& actor : actors) {
        const int32_t maxHp = readParamAtLevel(actor, 0, 100);
        const int32_t maxMp = readParamAtLevel(actor, 1, 30);
        actor.hp = std::clamp(actor.hp, 0, maxHp);
        actor.mp = std::clamp(actor.mp, 0, maxMp);
        actor.tp = std::clamp(actor.tp, 0, 100);
    }
}

} // namespace urpg::compat
