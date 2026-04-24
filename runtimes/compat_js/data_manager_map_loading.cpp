#include "runtimes/compat_js/data_manager.h"

#include "runtimes/compat_js/data_manager_internal_state.h"
#include "runtimes/compat_js/data_manager_support.h"

#include <utility>
#include <vector>

namespace urpg {
namespace compat {
bool DataManager::loadCommonEvents() {
    return true;
}

bool DataManager::loadSystem() {
    if (auto j = loadJsonFile(dataDirectory_ + "System.json")) {
        startMapId_ = j->value("startMapId", 1);
        startX_ = j->value("startX", 0);
        startY_ = j->value("startY", 0);
        startParty_.clear();
        if (j->contains("partyMembers") && (*j)["partyMembers"].is_array()) {
            for (const auto& m : (*j)["partyMembers"]) {
                startParty_.push_back(m.get<int32_t>());
            }
        }
        return true;
    }
    startMapId_ = 1;
    startX_ = 8;
    startY_ = 6;
    startParty_ = {1};
    return true;
}

bool DataManager::loadMapInfos() {
    mapInfos_.clear();
    if (auto j = loadJsonArrayFile("MapInfos.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            MapInfo info;
            info.id = elem.value("id", 0);
            info.name = elem.value("name", "");
            info.parentId = elem.value("parentId", 0);
            info.order = elem.value("order", 0);
            info.expanded = elem.value("expanded", false);
            if (info.id > 0 && static_cast<size_t>(info.id) > mapInfos_.size()) {
                mapInfos_.resize(static_cast<size_t>(info.id));
            }
            if (info.id > 0) {
                mapInfos_[static_cast<size_t>(info.id) - 1] = std::move(info);
            }
        }
        return true;
    }
    return true;
}

bool DataManager::loadMapData(int32_t mapId) {
    currentMap_ = MapData{};
    currentMap_.id = mapId;

    if (mapId > 0) {
        const std::string filename = buildMapFilename(mapId);
        if (auto j = loadJsonFile(dataDirectory_ + filename)) {
            currentMap_.width = j->value("width", 0);
            currentMap_.height = j->value("height", 0);
            currentMap_.tilesetId = j->value("tilesetId", 0);
            if (j->contains("data")) {
                currentMap_.data = convertMapLayers((*j)["data"], currentMap_.width, currentMap_.height);
            }
            if (j->contains("events")) {
                currentMap_.events = jsonToValue((*j)["events"]);
            } else {
                currentMap_.events = Value::Arr({});
            }

            impl_->loadedMapId = mapId;
            return true;
        }
    }

    currentMap_.width = 20;
    currentMap_.height = 15;
    currentMap_.tilesetId = 1;
    currentMap_.events = Value::Arr({});

    currentMap_.data.clear();
    currentMap_.data.resize(6, std::vector<int32_t>(300, 0));
    for (int i = 0; i < 300; ++i) {
        const int x = i % 20;
        const int y = i / 20;
        currentMap_.data[0][i] = (x == 0 || x == 19 || y == 0 || y == 14) ? 1 : 0;
    }

    impl_->loadedMapId = mapId;
    return true;
}

const MapData* DataManager::getCurrentMap() const {
    if (impl_->loadedMapId == 0) return nullptr;
    return &currentMap_;
}

Value DataManager::getMapDataAsValue() const {
    if (impl_->loadedMapId == 0) {
        return Value::Nil();
    }

    Object obj;
    obj["id"] = Value::Int(currentMap_.id);
    obj["width"] = Value::Int(currentMap_.width);
    obj["height"] = Value::Int(currentMap_.height);
    obj["tilesetId"] = Value::Int(currentMap_.tilesetId);

    Array layers;
    layers.reserve(currentMap_.data.size());
    for (const auto& layer : currentMap_.data) {
        Array layerValues;
        layerValues.reserve(layer.size());
        for (int32_t tileId : layer) {
            layerValues.push_back(Value::Int(tileId));
        }
        layers.push_back(Value::Arr(std::move(layerValues)));
    }
    obj["data"] = Value::Arr(std::move(layers));
    obj["events"] = currentMap_.events;
    return Value::Obj(std::move(obj));
}

int32_t DataManager::getStartMapId() const {
    return startMapId_;
}

int32_t DataManager::getStartX() const {
    return startX_;
}

int32_t DataManager::getStartY() const {
    return startY_;
}

int32_t DataManager::getStartPartySize() const {
    return static_cast<int32_t>(startParty_.size());
}

int32_t DataManager::getStartPartyMember(int32_t index) const {
    if (index < 0 || static_cast<size_t>(index) >= startParty_.size()) {
        return 0;
    }
    return startParty_[static_cast<size_t>(index)];
}
} // namespace compat
} // namespace urpg
