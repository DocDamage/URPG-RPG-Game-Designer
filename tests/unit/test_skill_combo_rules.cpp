#include "engine/core/ability/skill_combo_rules.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Skill combo rules trigger only for matching tags and known skills", "[ability][combo][ffs12]") {
    urpg::ability::SkillComboRules rules;
    rules.setKnownSkills({"skill.fire", "skill.wind"});
    rules.addRule({"combo.flame_gale", {"fire", "wind"}, {"skill.fire", "skill.wind"}, "burning_gale"});

    REQUIRE(rules.match({"fire", "wind"}, {"skill.fire", "skill.wind"}).has_value());
    REQUIRE_FALSE(rules.match({"fire"}, {"skill.fire", "skill.wind"}).has_value());
    REQUIRE(rules.validate().empty());

    rules.addRule({"combo.bad", {"void"}, {"skill.missing"}, "bad"});
    REQUIRE(rules.validate().back().code == "missing_combo_skill");
}
