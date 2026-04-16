#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/health_components.h"
#include "engine/core/ui/ui_components.h"

namespace urpg {

/**
 * @brief System that synchronizes gameplay state (like health) to UI components.
 */
class UIStateSyncSystem {
public:
    void update(World& world) {
        // Sync Health to Progress Bars
        world.ForEachWith<HealthComponent, UIProgressBarComponent>([&](EntityID, const HealthComponent& health, UIProgressBarComponent& bar) {
            if (health.max > 0) {
                float ratio = static_cast<float>(health.current) / static_cast<float>(health.max);
                bar.progress = Fixed32::FromRaw(static_cast<int32_t>(ratio * 65536.0f));
            }
        });

        // Sync Health to Text Bindings
        world.ForEachWith<HealthComponent, UIWidgetComponent>([&](EntityID, const HealthComponent& health, UIWidgetComponent& ui) {
            for (auto& binding : ui.bindings) {
                if (binding.key == "hp_text") {
                    binding.value = std::to_string(health.current) + "/" + std::to_string(health.max);
                }
            }
        });
    }
};

} // namespace urpg
