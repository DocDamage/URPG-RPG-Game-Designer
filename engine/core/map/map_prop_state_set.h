#pragma once

#include "engine/core/map/grid_part_types.h"

#include <string>
#include <vector>

namespace urpg::map {

struct MapPropState {
    std::string id;
    std::string attachedAssetId;
    GridPartFootprint collision;
    GridPartLayer layer = GridPartLayer::Object;
    std::string interactionEventId;
};

struct MapPropStateValidation {
    bool valid = false;
    std::vector<std::string> codes;
};

class MapPropStateSet {
  public:
    explicit MapPropStateSet(std::string propId = {});
    bool addState(MapPropState state);
    bool setActiveState(const std::string& stateId);
    const MapPropState* activeState() const;
    MapPropStateValidation validate(const std::vector<std::string>& knownAssetIds,
                                    const std::vector<std::string>& knownEventIds) const;
    bool applyTo(PlacedPartInstance& part) const;

  private:
    std::string propId_;
    std::string activeStateId_;
    std::vector<MapPropState> states_;
};

} // namespace urpg::map
