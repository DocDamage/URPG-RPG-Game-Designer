// INCUBATING TEST: This file contains a standalone main() and is not yet
// integrated into the Catch2 test suite (urpg_tests). Do not register it in
// CMakeLists.txt until it is converted to Catch2 TEST_CASE macros.
//
#include "engine/core/ability/pattern_field.h"
#include "engine/core/ability/pattern_field_serializer.h"
#include <iostream>
#include <cassert>

using namespace urpg;

int main() {
    std::cout << "Testing Pattern Field Serialization...\n";

    // 1. Create a pattern
    PatternField original("Fireball AoE");
    original.addPoint(0, 0);
    original.addPoint(1, 0);
    original.addPoint(-1, 0);
    original.addPoint(0, 1);
    original.addPoint(0, -1);

    // 2. Serialize to JSON
    nlohmann::json j = PatternFieldSerializer::toJson(original);
    std::cout << "\nSerialized JSON:\n" << j.dump(4) << "\n";

    // 3. Deserialize from JSON
    PatternField restored = PatternFieldSerializer::fromJson(j);

    // 4. Verify
    assert(restored.getName() == "Fireball AoE");
    assert(restored.getPoints().size() == 5);
    assert(restored.hasPoint(0, 0));
    assert(restored.hasPoint(1, 0));
    assert(restored.hasPoint(-1, 0));
    assert(restored.hasPoint(0, 1));
    assert(restored.hasPoint(0, -1));
    assert(!restored.hasPoint(1, 1));

    std::cout << "\nPattern Field Serialization test completed successfully.\n";
    return 0;
}
