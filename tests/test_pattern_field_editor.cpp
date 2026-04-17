// INCUBATING TEST: This file contains a standalone main() and is not yet
// integrated into the Catch2 test suite (urpg_tests). Do not register it in
// CMakeLists.txt until it is converted to Catch2 TEST_CASE macros.
//
#include "editor/ability/pattern_field_model.h"
#include "editor/ability/pattern_field_panel.h"
#include "engine/core/ability/pattern_field.h"
#include <iostream>
#include <cassert>

using namespace urpg;
using namespace urpg::editor;

int main() {
    std::cout << "Testing Pattern Field Editor (Manual Painting Simulation)...\n";

    PatternFieldModel model;
    PatternFieldPanel panel;

    // 1. Initial State (Empty 5x5 by default)
    panel.update(model);
    std::cout << "\n[Initial Empty Grid]\n";
    panel.render();

    // 2. Add some points (AoE burst)
    model.togglePoint(0, 0); // Origin
    model.togglePoint(1, 0);
    model.togglePoint(-1, 0);
    model.togglePoint(0, 1);
    model.togglePoint(0, -1);
    
    panel.update(model);
    std::cout << "\n[Painted Cross Pattern (+)]\n";
    panel.render();
    assert(model.isPointSelected(0, 0));
    assert(model.isPointSelected(1, 0));
    assert(model.isPointSelected(-1, 0));
    assert(!model.isPointSelected(1, 1));

    // 3. Remove a point
    model.togglePoint(1, 0);
    panel.update(model);
    std::cout << "\n[Removed Point (1,0)]\n";
    panel.render();
    assert(!model.isPointSelected(1, 0));

    // 4. Test 7x7 viewport
    model.setViewportSize(7);
    panel.update(model);
    std::cout << "\n[7x7 Viewport]\n";
    panel.render();

    std::cout << "Visual Pattern Editor test completed successfully.\n";
    return 0;
}
