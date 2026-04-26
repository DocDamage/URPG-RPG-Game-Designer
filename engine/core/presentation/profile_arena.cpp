#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "presentation_arena.h"
#include <vector>

/**
 * @brief Simple simulation to profile Phase 1 arena allocation costs.
 *
 * Based on Section 19 and 21 of the plan.
 */
int main() {
    using namespace urpg::presentation;
    using urpg::diagnostics::RuntimeDiagnostics;

    // Standard Tier 1 Arena (2MB as per performance_budgets.md)
    size_t tier1Capacity = 2 * 1024 * 1024;
    PresentationArena arena(tier1Capacity);

    RuntimeDiagnostics::info("presentation.profile_arena", "profile_arena.simulation_started",
                             "Starting Simulation: Representative MapScene (Tier 1)");
    RuntimeDiagnostics::info("presentation.profile_arena", "profile_arena.initial_capacity",
                             "Initial Capacity: " + std::to_string(arena.GetCapacity()) + " bytes");

    // Simulate allocating a frame's intent data
    // 1000 Tile commands (~64 bytes each)
    // 100 Sprite commands (~128 bytes each)
    // Lights, Camera, etc.

    struct DummyTileCmd {
        uint32_t id;
        float x, y, z;
        uint8_t data[44];
    };
    struct DummySpriteCmd {
        uint32_t id;
        float transform[16];
        uint8_t data[60];
    };

    for (int i = 0; i < 1000; ++i) {
        arena.Allocate(sizeof(DummyTileCmd));
    }
    for (int i = 0; i < 100; ++i) {
        arena.Allocate(sizeof(DummySpriteCmd));
    }

    RuntimeDiagnostics::info("presentation.profile_arena", "profile_arena.used_bytes",
                             "Used after simulate tiles/sprites: " + std::to_string(arena.GetUsedBytes()) + " bytes");

    if (arena.GetUsedBytes() < tier1Capacity) {
        RuntimeDiagnostics::info("presentation.profile_arena", "profile_arena.pass",
                                 "Profile PASS: Representative frame fits in Tier 1 budget.");
    } else {
        RuntimeDiagnostics::error("presentation.profile_arena", "profile_arena.fail", "Profile FAIL: Budget breach.");
    }

    return 0;
}
