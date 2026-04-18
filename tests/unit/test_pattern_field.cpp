#include <catch2/catch_test_macros.hpp>
#include "engine/core/ability/pattern_field.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include <nlohmann/json.hpp>

using namespace urpg::ability;
using namespace urpg;

class TestAbilityWithPattern : public GameplayAbility {
public:
    TestAbilityWithPattern(std::shared_ptr<PatternField> p) {
        m_info.pattern = p;
    }
    const std::string& getId() const override { static std::string id = "TestPat"; return id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }
    void activate(AbilitySystemComponent& source) override {}
private:
    ActivationInfo m_info;
};

TEST_CASE("PatternField: Logic and Serialization", "[ability][pattern]") {
    PatternField pattern("Burst");
    pattern.addPoint(1, 0);
    pattern.addPoint(-1, 0);
    pattern.addPoint(0, 1);
    pattern.addPoint(0, -1);

    SECTION("Point containment") {
        REQUIRE(pattern.hasPoint(1, 0));
        REQUIRE(pattern.hasPoint(0, -1));
        REQUIRE_FALSE(pattern.hasPoint(1, 1));
        REQUIRE_FALSE(pattern.hasPoint(2, 0));
    }

    SECTION("Bounds calculation") {
        int32_t minX, minY, maxX, maxY;
        pattern.getBounds(minX, minY, maxX, maxY);
        REQUIRE(minX == -1);
        REQUIRE(maxX == 1);
        REQUIRE(minY == -1);
        REQUIRE(maxY == 1);
    }

    SECTION("JSON round-trip") {
        nlohmann::json j = pattern;
        REQUIRE(j["name"] == "Burst");
        REQUIRE(j["points"].size() == 4);

        PatternField pattern2 = j.get<PatternField>();
        REQUIRE(pattern2.getName() == "Burst");
        REQUIRE(pattern2.hasPoint(1, 0));
        REQUIRE(pattern2.hasPoint(0, 1));
    }
}

TEST_CASE("PatternValidator: Rules", "[ability][pattern][validation]") {
    SECTION("Empty pattern is invalid") {
        PatternField empty("Empty");
        auto result = PatternValidator::Validate(empty);
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.issues.size() == 1);
        REQUIRE(result.issues[0] == "Pattern contains no points.");
    }

    SECTION("Out of bounds pattern is invalid") {
        PatternField huge("Huge");
        huge.addPoint(50, 50);
        auto result = PatternValidator::Validate(huge, 10);
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.issues.size() == 1);
    }

    SECTION("Normal pattern is valid") {
        PatternField normal("Normal");
        normal.addPoint(1, 1);
        auto result = PatternValidator::Validate(normal);
        REQUIRE(result.isValid);
        REQUIRE(result.issues.empty());
    }
}

TEST_CASE("AbilitySystem: Pattern Validation", "[ability][pattern]") {
    auto pattern = std::make_shared<PatternField>("Line");
    pattern->addPoint(1, 0);
    pattern->addPoint(2, 0);

    TestAbilityWithPattern ability(pattern);
    AbilitySystemComponent asc;

    SECTION("Target within pattern") {
        // Source at (10, 10), Target at (11, 10) -> dx=1, dy=0
        REQUIRE(asc.isTargetInPattern(ability, 10, 10, 11, 10));
        // dx=2, dy=0
        REQUIRE(asc.isTargetInPattern(ability, 10, 10, 12, 10));
    }

    SECTION("Target outside pattern") {
        // dx=0, dy=0 (Origin not in this specific pattern)
        REQUIRE_FALSE(asc.isTargetInPattern(ability, 10, 10, 10, 10));
        // dx=3, dy=0
        REQUIRE_FALSE(asc.isTargetInPattern(ability, 10, 10, 13, 10));
        // dx=1, dy=1
        REQUIRE_FALSE(asc.isTargetInPattern(ability, 10, 10, 11, 11));
    }
}
