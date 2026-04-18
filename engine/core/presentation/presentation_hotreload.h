#pragma once

#include "presentation_schema.h"
#include <map>
#include <string>

namespace urpg::presentation {

/**
 * @brief Asset Registry for presentation profiles.
 * In a real engine, this would use handles, but for the spine validation, 
 * we use string-based lookup.
 */
class PresentationRegistry {
public:
    void RegisterMapOverlay(const std::string& id, const SpatialMapOverlay& overlay) {
        m_mapOverlays[id] = overlay;
    }

    void RegisterActorProfile(const std::string& id, const ActorPresentationProfile& profile) {
        m_actorProfiles[id] = profile;
    }

    void RegisterBattleConfig(const std::string& id, const BattlePresentationConfig& config) {
        m_battleConfigs[id] = config;
    }

    const SpatialMapOverlay* GetMapOverlay(const std::string& id) const {
        auto it = m_mapOverlays.find(id);
        return it != m_mapOverlays.end() ? &it->second : nullptr;
    }

    const ActorPresentationProfile* GetActorProfile(const std::string& id) const {
        auto it = m_actorProfiles.find(id);
        return it != m_actorProfiles.end() ? &it->second : nullptr;
    }

    const BattlePresentationConfig* GetBattleConfig(const std::string& id) const {
        auto it = m_battleConfigs.find(id);
        return it != m_battleConfigs.end() ? &it->second : nullptr;
    }

private:
    std::map<std::string, SpatialMapOverlay> m_mapOverlays;
    std::map<std::string, ActorPresentationProfile> m_actorProfiles;
    std::map<std::string, BattlePresentationConfig> m_battleConfigs;
};

/**
 * @brief Infrastructure for hot-reloading presentation data.
 */
class PresentationHotReloader {
public:
    virtual ~PresentationHotReloader() = default;
    
    /**
     * @brief Triggered when an asset file changes on disk.
     */
    virtual void OnAssetChanged(const std::string& assetPath) = 0;
    
    /**
     * @brief Poll for requested reloads.
     */
    virtual bool Update() = 0;
};

} // namespace urpg::presentation
