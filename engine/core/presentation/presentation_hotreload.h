#pragma once

#include "presentation_schema.h"
#include "presentation_arena.h"
#include <map>
#include <string>
#include <vector>
#include <functional>

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

    void Clear() {
        m_mapOverlays.clear();
        m_actorProfiles.clear();
        m_battleConfigs.clear();
    }

    size_t MapOverlayCount() const { return m_mapOverlays.size(); }
    size_t ActorProfileCount() const { return m_actorProfiles.size(); }

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
     * @brief Poll for requested reloads. Returns true if any reload occurred.
     */
    virtual bool Update() = 0;
};

/**
 * @brief Callback type invoked when a profile asset is reloaded.
 * Receives the changed asset path and the (cleared) registry for re-population.
 */
using ProfileReloadCallback = std::function<void(const std::string& assetPath, PresentationRegistry& registry)>;

/**
 * @brief Concrete hot-reloader that resets a ProfileArena between reload cycles.
 *
 * Usage:
 *   - Register a callback via SetReloadCallback() that re-populates the registry.
 *   - Call OnAssetChanged() when a profile asset changes on disk.
 *   - Call Update() each frame; returns true when a reload was processed.
 *   - The arena is reset (not reallocated) on each reload, preventing resource leaks.
 *
 * S23-T02: Registered as the production hot-reload path for the Presentation Core.
 */
class ProfileArenaHotReloader final : public PresentationHotReloader {
public:
    /**
     * @param arenaCapacityBytes  Size of the per-reload profile arena (default: 512 KB).
     */
    explicit ProfileArenaHotReloader(size_t arenaCapacityBytes = 512 * 1024)
        : m_arena(arenaCapacityBytes) {}

    ~ProfileArenaHotReloader() override = default;

    /**
     * @brief Register the callback used to re-populate the registry on reload.
     */
    void SetReloadCallback(ProfileReloadCallback callback) {
        m_reloadCallback = std::move(callback);
    }

    /**
     * @brief Queue an asset path for reload on the next Update() tick.
     */
    void OnAssetChanged(const std::string& assetPath) override {
        for (const auto& pending : m_pendingPaths) {
            if (pending == assetPath) return; // deduplicate
        }
        m_pendingPaths.push_back(assetPath);
    }

    /**
     * @brief Process all pending reload requests.
     *
     * For each pending path:
     *   1. The arena is reset (zero-cost pointer bump back to start).
     *   2. The registry is cleared.
     *   3. The registered callback is invoked to re-populate the registry.
     *
     * Returns true if at least one reload was processed this tick.
     */
    bool Update() override {
        if (m_pendingPaths.empty()) return false;

        for (const auto& path : m_pendingPaths) {
            // Reset arena — no allocation is freed, just the offset is reset.
            m_arena.Reset();
            m_registry.Clear();
            ++m_reloadCount;

            if (m_reloadCallback) {
                m_reloadCallback(path, m_registry);
            }
        }
        m_pendingPaths.clear();
        return true;
    }

    /**
     * @brief Read-only access to the live registry after reload.
     */
    const PresentationRegistry& GetRegistry() const { return m_registry; }

    /**
     * @brief Number of reload cycles completed since construction.
     * Used to invalidate downstream caches.
     */
    uint32_t GetReloadCount() const { return m_reloadCount; }

    /**
     * @brief Arena usage after the most recent reload.
     * Useful for budget diagnostics.
     */
    size_t GetArenaUsedBytes() const { return m_arena.GetUsedBytes(); }
    size_t GetArenaCapacityBytes() const { return m_arena.GetCapacity(); }

private:
    PresentationArena m_arena;
    PresentationRegistry m_registry;
    ProfileReloadCallback m_reloadCallback;
    std::vector<std::string> m_pendingPaths;
    uint32_t m_reloadCount = 0;
};

} // namespace urpg::presentation
