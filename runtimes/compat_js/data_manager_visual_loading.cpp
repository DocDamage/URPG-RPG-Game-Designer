#include "runtimes/compat_js/data_manager.h"

#include "runtimes/compat_js/data_manager_support.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace urpg {
namespace compat {
bool DataManager::loadStates() {
    states_.clear();
    if (auto j = loadJsonArrayFile("States.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            StateData state;
            state.id = elem.value("id", 0);
            state.name = elem.value("name", "");
            if (elem.contains("iconIndex")) {
                if (elem["iconIndex"].is_number()) {
                    state.iconIndex = std::to_string(elem["iconIndex"].get<int32_t>());
                } else {
                    state.iconIndex = elem.value("iconIndex", "");
                }
            }
            state.priority = elem.value("priority", 0);
            state.restriction = elem.value("restriction", 0);
            state.autoRemovalTiming = elem.value("autoRemovalTiming", 0);
            state.minTurns = elem.value("minTurns", 1);
            state.maxTurns = elem.value("maxTurns", 1);
            if (state.id > 0 && static_cast<size_t>(state.id) > states_.size()) {
                states_.resize(static_cast<size_t>(state.id));
            }
            if (state.id > 0) {
                states_[static_cast<size_t>(state.id) - 1] = std::move(state);
            }
        }
        return true;
    }
    return true;
}

bool DataManager::loadAnimations() {
    animations_.clear();
    if (auto j = loadJsonArrayFile("Animations.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            AnimationData anim;
            anim.id = elem.value("id", 0);
            anim.name = elem.value("name", "");
            if (elem.contains("frames") && elem["frames"].is_array()) {
                for (const auto& f : elem["frames"]) {
                    anim.frames.push_back(jsonToValue(f));
                }
            }
            if (anim.id > 0 && static_cast<size_t>(anim.id) > animations_.size()) {
                animations_.resize(static_cast<size_t>(anim.id));
            }
            if (anim.id > 0) {
                animations_[static_cast<size_t>(anim.id) - 1] = std::move(anim);
            }
        }
        return true;
    }
    auto addFallbackAnimation = [this](int32_t id, const std::string& name, int32_t frameCount) {
        AnimationData anim;
        anim.id = id;
        anim.name = name;
        anim.frames.resize(static_cast<size_t>(std::max(1, frameCount)), Value::Nil());
        if (static_cast<size_t>(id) > animations_.size()) {
            animations_.resize(static_cast<size_t>(id));
        }
        animations_[static_cast<size_t>(id) - 1] = std::move(anim);
    };
    addFallbackAnimation(41, "Heal", 10);
    addFallbackAnimation(67, "Fire", 12);
    return true;
}

bool DataManager::loadTilesets() {
    tilesets_.clear();
    if (auto j = loadJsonArrayFile("Tilesets.json")) {
        for (const auto& elem : *j) {
            if (elem.is_null()) continue;
            TilesetData ts;
            ts.id = elem.value("id", 0);
            ts.name = elem.value("name", "");
            ts.mode = elem.value("mode", 0);
            if (elem.contains("tilesetNames") && elem["tilesetNames"].is_array()) {
                for (const auto& t : elem["tilesetNames"]) {
                    ts.tilesetNames.push_back(t.get<std::string>());
                }
            }
            if (elem.contains("flags") && elem["flags"].is_array()) {
                for (const auto& f : elem["flags"]) {
                    ts.flags.push_back(f.get<uint32_t>());
                }
            }
            if (ts.id > 0 && static_cast<size_t>(ts.id) > tilesets_.size()) {
                tilesets_.resize(static_cast<size_t>(ts.id));
            }
            if (ts.id > 0) {
                tilesets_[static_cast<size_t>(ts.id) - 1] = std::move(ts);
            }
        }
        return true;
    }
    return true;
}
} // namespace compat
} // namespace urpg
