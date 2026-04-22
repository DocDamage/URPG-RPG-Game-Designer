#include <catch2/catch_test_macros.hpp>
#include "engine/core/ability/pattern_field.h"
#include "engine/core/ability/gameplay_ability.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/pattern_field_serializer.h"
#include "engine/core/ability/pattern_field_presets.h"
#include "editor/ability/pattern_field_model.h"
#include "editor/ability/pattern_field_panel.h"
#include <nlohmann/json.hpp>
#include <algorithm>

using namespace urpg::ability;
using namespace urpg;

class TestAbilityWithPattern : public GameplayAbility {
public:
    TestAbilityWithPattern(std::shared_ptr<PatternField> p) {
        m_info.pattern = p;
    }
    const std::string& getId() const override { static std::string id = "TestPat"; return id; }
    const ActivationInfo& getActivationInfo() const override { return m_info; }
    void activate([[maybe_unused]] AbilitySystemComponent& source) override {}
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
        REQUIRE(result.issues.size() >= 1);
        REQUIRE(std::find(result.issues.begin(),
                          result.issues.end(),
                          "Pattern exceeds maximum radius of 10.") != result.issues.end());
    }

    SECTION("Normal pattern is valid") {
        PatternField normal("Normal");
        normal.addPoint(0, 0);
        normal.addPoint(1, 1);
        auto result = PatternValidator::Validate(normal);
        REQUIRE(result.isValid);
        REQUIRE(result.issues.empty());
    }

    SECTION("Pattern without origin is invalid") {
        PatternField offset("Offset");
        offset.addPoint(1, 0);
        auto result = PatternValidator::Validate(offset);
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.issues.size() == 1);
        REQUIRE(result.issues[0] == "Pattern must include the origin point (0,0).");
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

TEST_CASE("PatternFieldModel applies deterministic presets and preview snapshots", "[ability][pattern][editor]") {
    urpg::editor::PatternFieldModel model;
    model.resizeViewport(5);
    model.applyPreset("cross_small");

    const auto pattern = model.getCurrentPattern();
    REQUIRE(pattern);
    REQUIRE(pattern->getName() == "Cross Small");
    REQUIRE(pattern->hasPoint(0, 0));
    REQUIRE(pattern->hasPoint(1, 0));
    REQUIRE(pattern->hasPoint(-1, 0));
    REQUIRE(pattern->hasPoint(0, 1));
    REQUIRE(pattern->hasPoint(0, -1));

    const auto preview = model.buildPreviewSnapshot();
    REQUIRE(preview.is_valid);
    REQUIRE(preview.issues.empty());
    REQUIRE(preview.grid_rows.size() == 5);
    REQUIRE(preview.grid_rows[2].find("[O]") != std::string::npos);
    REQUIRE(preview.grid_rows[1].find("[X]") != std::string::npos);
}

TEST_CASE("PatternField preset catalog exposes reusable domains for skills, items, placement, and interaction masks",
          "[ability][pattern][presets]") {
    const auto catalog = urpg::PatternFieldPresets::Catalog();
    REQUIRE(catalog.size() >= 4);

    const auto skill = urpg::PatternFieldPresets::FindById("skill_cross_small");
    REQUIRE(skill.has_value());
    REQUIRE(skill->usage == urpg::PatternFieldPresets::Usage::Skill);
    REQUIRE(skill->pattern.hasPoint(0, 0));
    REQUIRE(skill->pattern.hasPoint(1, 0));

    const auto item = urpg::PatternFieldPresets::FindById("item_blast_square");
    REQUIRE(item.has_value());
    REQUIRE(item->usage == urpg::PatternFieldPresets::Usage::Item);
    REQUIRE(item->pattern.hasPoint(-1, -1));
    REQUIRE(item->pattern.hasPoint(1, 1));

    const auto placement = urpg::PatternFieldPresets::FindById("placement_pad");
    REQUIRE(placement.has_value());
    REQUIRE(placement->usage == urpg::PatternFieldPresets::Usage::Placement);
    REQUIRE(placement->pattern.hasPoint(1, 1));

    const auto interaction = urpg::PatternFieldPresets::FindById("interaction_mask_doorway");
    REQUIRE(interaction.has_value());
    REQUIRE(interaction->usage == urpg::PatternFieldPresets::Usage::InteractionMask);
    REQUIRE(interaction->pattern.hasPoint(-1, 0));
    REQUIRE(interaction->pattern.hasPoint(1, 0));
}

TEST_CASE("PatternFieldModel filters reusable preset catalog by usage and applies categorized presets",
          "[ability][pattern][editor]") {
    urpg::editor::PatternFieldModel model;

    const auto placementPresets =
        model.availablePresets(urpg::PatternFieldPresets::Usage::Placement);
    REQUIRE(placementPresets.size() == 1);
    REQUIRE(placementPresets[0].id == "placement_pad");

    model.applyPreset("interaction_mask_doorway");
    const auto pattern = model.getCurrentPattern();
    REQUIRE(pattern);
    REQUIRE(pattern->getName() == "Interaction Doorway");
    REQUIRE(pattern->hasPoint(-1, 0));
    REQUIRE(pattern->hasPoint(0, 0));
    REQUIRE(pattern->hasPoint(1, 0));
}

TEST_CASE("PatternFieldPanel snapshot carries preview and validation issues", "[ability][pattern][panel]") {
    urpg::editor::PatternFieldModel model;
    auto pattern = std::make_shared<PatternField>("Offset");
    pattern->addPoint(1, 0);
    model.setCurrentPattern(pattern);
    model.resizeViewport(3);

    urpg::editor::PatternFieldPanel panel;
    panel.update(model);

    const auto& snapshot = panel.getRenderSnapshot();
    REQUIRE(snapshot.visible);
    REQUIRE(snapshot.name == "Offset");
    REQUIRE_FALSE(snapshot.is_valid);
    REQUIRE(snapshot.issues.size() == 1);
    REQUIRE(snapshot.issues[0] == "Pattern must include the origin point (0,0).");
    REQUIRE(snapshot.grid_rows.size() == 3);
}

TEST_CASE("PatternFieldSerializer preserves deterministic point ordering", "[ability][pattern][serializer]") {
    PatternField pattern("Ordered");
    pattern.addPoint(1, 0);
    pattern.addPoint(0, 0);
    pattern.addPoint(-1, 0);

    const auto json = urpg::PatternFieldSerializer::toJson(pattern);
    REQUIRE(json["points"][0]["x"] == -1);
    REQUIRE(json["points"][1]["x"] == 0);
    REQUIRE(json["points"][2]["x"] == 1);

    const auto roundTrip = urpg::PatternFieldSerializer::fromJson(json);
    REQUIRE(roundTrip.hasPoint(-1, 0));
    REQUIRE(roundTrip.hasPoint(0, 0));
    REQUIRE(roundTrip.hasPoint(1, 0));
}
