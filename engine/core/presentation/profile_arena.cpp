#include "presentation_arena.h"
#include <iostream>
#include <vector>

/**
 * @brief Simple simulation to profile Phase 1 arena allocation costs.
 * 
 * Based on Section 19 and 21 of the plan.
 */
int main() {
    using namespace urpg::presentation;

    // Standard Tier 1 Arena (2MB as per performance_budgets.md)
    size_t tier1Capacity = 2 * 1024 * 1024;
    PresentationArena arena(tier1Capacity);

    std::cout << "Starting Simulation: Representative MapScene (Tier 1)" << std::endl;
    std::cout << "Initial Capacity: " << arena.GetCapacity() << " bytes" << std::endl;

    // Simulate allocating a frame's intent data
    // 1000 Tile commands (~64 bytes each)
    // 100 Sprite commands (~128 bytes each)
    // Lights, Camera, etc.
    
    struct DummyTileCmd { uint32_t id; float x, y, z; uint8_t data[44]; };
    struct DummySpriteCmd { uint32_t id; float transform[16]; uint8_t data[60]; };

    for(int i = 0; i < 1000; ++i) {
        arena.Allocate(sizeof(DummyTileCmd));
    }
    for(int i = 0; i < 100; ++i) {
        arena.Allocate(sizeof(DummySpriteCmd));
    }

    std::cout << "Used after simulate tiles/sprites: " << arena.GetUsedBytes() << " bytes" << std::endl;
    
    if (arena.GetUsedBytes() < tier1Capacity) {
        std::cout << "Profile PASS: Representative frame fits in Tier 1 budget." << std::endl;
    } else {
        std::cout << "Profile FAIL: Budget breach!" << std::endl;
    }

    return 0;
}
