#include "engine/core/map/map_prop_state_set.h"

#include <algorithm>

namespace urpg::map {

MapPropStateSet::MapPropStateSet(std::string propId) : propId_(std::move(propId)) {}

bool MapPropStateSet::addState(MapPropState state) {
    if (state.id.empty() || std::any_of(states_.begin(), states_.end(), [&](const auto& item) { return item.id == state.id; })) {
        return false;
    }
    states_.push_back(std::move(state));
    if (activeStateId_.empty()) activeStateId_ = states_.back().id;
    return true;
}

bool MapPropStateSet::setActiveState(const std::string& stateId) {
    if (std::none_of(states_.begin(), states_.end(), [&](const auto& item) { return item.id == stateId; })) return false;
    activeStateId_ = stateId;
    return true;
}

const MapPropState* MapPropStateSet::activeState() const {
    const auto found = std::find_if(states_.begin(), states_.end(), [&](const auto& item) { return item.id == activeStateId_; });
    return found == states_.end() ? nullptr : &*found;
}

MapPropStateValidation MapPropStateSet::validate(const std::vector<std::string>& knownAssetIds,
                                                 const std::vector<std::string>& knownEventIds) const {
    MapPropStateValidation result;
    if (propId_.empty()) result.codes.push_back("prop_state_missing_prop_id");
    if (states_.empty()) result.codes.push_back("prop_state_missing_states");
    for (const auto& state : states_) {
        if (state.attachedAssetId.empty() || std::find(knownAssetIds.begin(), knownAssetIds.end(), state.attachedAssetId) == knownAssetIds.end()) {
            result.codes.push_back("prop_state_unattached_asset:" + state.id);
        }
        if (!state.interactionEventId.empty() &&
            std::find(knownEventIds.begin(), knownEventIds.end(), state.interactionEventId) == knownEventIds.end()) {
            result.codes.push_back("prop_state_unknown_event:" + state.id);
        }
    }
    result.valid = result.codes.empty();
    return result;
}

bool MapPropStateSet::applyTo(PlacedPartInstance& part) const {
    const auto* state = activeState();
    if (state == nullptr || part.instance_id.empty()) return false;
    part.layer = state->layer;
    part.width = state->collision.width;
    part.height = state->collision.height;
    part.properties["propState"] = state->id;
    part.properties["assetId"] = state->attachedAssetId;
    part.properties["interactionEventId"] = state->interactionEventId;
    part.properties["blocksNavigation"] = state->collision.blocks_navigation ? "true" : "false";
    return true;
}

} // namespace urpg::map
