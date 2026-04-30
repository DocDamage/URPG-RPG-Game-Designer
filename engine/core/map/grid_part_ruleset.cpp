#include "engine/core/map/grid_part_ruleset.h"

#include <algorithm>
#include <string>
#include <utility>

namespace urpg::map {

namespace {

GridPartDiagnostic makeRulesetDiagnostic(GridPartSeverity severity, std::string code, std::string message,
                                         const PlacedPartInstance* instance = nullptr, std::string target = {}) {
    GridPartDiagnostic diagnostic;
    diagnostic.severity = severity;
    diagnostic.code = std::move(code);
    diagnostic.message = std::move(message);
    diagnostic.target = std::move(target);
    if (instance != nullptr) {
        diagnostic.instance_id = instance->instance_id;
        diagnostic.part_id = instance->part_id;
        diagnostic.x = instance->grid_x;
        diagnostic.y = instance->grid_y;
    }
    return diagnostic;
}

bool supportsRuleset(const GridPartDefinition& definition, GridPartRuleset ruleset) {
    return std::find(definition.supported_rulesets.begin(), definition.supported_rulesets.end(), ruleset) !=
           definition.supported_rulesets.end();
}

bool hasPropertyValue(const PlacedPartInstance& instance, const std::string& key, const std::string& value) {
    const auto found = instance.properties.find(key);
    return found != instance.properties.end() && found->second == value;
}

bool isSpawnPart(const PlacedPartInstance& instance) {
    return hasPropertyValue(instance, "role", "player_spawn") || hasPropertyValue(instance, "spawn", "player") ||
           instance.part_id.find("player_spawn") != std::string::npos ||
           instance.part_id.find("spawn.player") != std::string::npos;
}

bool isExitPart(const PlacedPartInstance& instance) {
    return hasPropertyValue(instance, "role", "exit") || hasPropertyValue(instance, "objective", "exit") ||
           instance.part_id.find(".exit") != std::string::npos || instance.part_id.find("exit.") != std::string::npos;
}

bool isGravityPart(const PlacedPartInstance& instance) {
    return hasPropertyValue(instance, "gravity", "true") || hasPropertyValue(instance, "requiresGravity", "true");
}

void sortDiagnostics(std::vector<GridPartDiagnostic>& diagnostics) {
    std::sort(diagnostics.begin(), diagnostics.end(),
              [](const GridPartDiagnostic& left, const GridPartDiagnostic& right) {
                  if (left.severity != right.severity) {
                      return static_cast<uint8_t>(left.severity) > static_cast<uint8_t>(right.severity);
                  }
                  if (left.code != right.code) {
                      return left.code < right.code;
                  }
                  if (left.instance_id != right.instance_id) {
                      return left.instance_id < right.instance_id;
                  }
                  return left.target < right.target;
              });
}

} // namespace

GridRulesetProfile MakeDefaultGridRulesetProfile(GridPartRuleset ruleset) {
    GridRulesetProfile profile;
    profile.ruleset = ruleset;

    switch (ruleset) {
    case GridPartRuleset::TopDownJRPG:
        profile.id = "top_down_jrpg";
        profile.display_name = "Top-Down JRPG";
        break;
    case GridPartRuleset::SideScrollerAction:
        profile.id = "side_scroller_action";
        profile.display_name = "Side-Scroller Action";
        profile.requires_exit = true;
        profile.allows_gravity = true;
        profile.allows_platforms = true;
        profile.uses_physics_collision = true;
        profile.allows_multiple_object_layers = false;
        break;
    case GridPartRuleset::TacticalGrid:
        profile.id = "tactical_grid";
        profile.display_name = "Tactical Grid";
        profile.uses_tactical_cover = true;
        profile.allows_freeform_props = false;
        break;
    case GridPartRuleset::DungeonRoomBuilder:
        profile.id = "dungeon_room_builder";
        profile.display_name = "Dungeon Room Builder";
        profile.requires_exit = true;
        profile.max_width = 128;
        profile.max_height = 128;
        break;
    case GridPartRuleset::WorldMap:
        profile.id = "world_map";
        profile.display_name = "World Map";
        profile.requires_player_spawn = false;
        profile.max_width = 512;
        profile.max_height = 512;
        break;
    case GridPartRuleset::TownHub:
        profile.id = "town_hub";
        profile.display_name = "Town Hub";
        profile.requires_exit = false;
        break;
    case GridPartRuleset::BattleArena:
        profile.id = "battle_arena";
        profile.display_name = "Battle Arena";
        profile.requires_exit = true;
        profile.max_width = 64;
        profile.max_height = 64;
        break;
    case GridPartRuleset::CutsceneStage:
        profile.id = "cutscene_stage";
        profile.display_name = "Cutscene Stage";
        profile.requires_player_spawn = false;
        profile.requires_exit = false;
        profile.uses_tile_passability = false;
        break;
    }

    return profile;
}

GridPartValidationResult ValidateGridPartRuleset(const GridPartDocument& document, const GridPartCatalog& catalog,
                                                 const GridRulesetProfile& profile) {
    GridPartValidationResult result;

    if (profile.id.empty()) {
        result.diagnostics.push_back(makeRulesetDiagnostic(GridPartSeverity::Blocker, "ruleset_missing",
                                                           "Grid ruleset profile is missing an id."));
    }

    if (document.width() > profile.max_width || document.height() > profile.max_height) {
        result.diagnostics.push_back(makeRulesetDiagnostic(GridPartSeverity::Error, "map_exceeds_ruleset_size",
                                                           "Grid part document exceeds the ruleset size limits.",
                                                           nullptr, profile.id));
    }

    bool hasSpawn = false;
    bool hasExit = false;
    for (const auto& instance : document.parts()) {
        hasSpawn = hasSpawn || isSpawnPart(instance);
        hasExit = hasExit || isExitPart(instance);

        const auto* definition = catalog.find(instance.part_id);
        if (definition != nullptr && !supportsRuleset(*definition, profile.ruleset)) {
            result.diagnostics.push_back(makeRulesetDiagnostic(
                GridPartSeverity::Error, "part_ruleset_incompatible",
                "Placed grid part is not supported by the requested ruleset.", &instance, definition->part_id));
        }

        if (!profile.allows_platforms && instance.category == GridPartCategory::Platform) {
            result.diagnostics.push_back(makeRulesetDiagnostic(
                GridPartSeverity::Error, "platform_not_allowed_in_ruleset",
                "Platform parts are not allowed by the requested ruleset.", &instance, profile.id));
        }

        if (!profile.allows_gravity && isGravityPart(instance)) {
            result.diagnostics.push_back(makeRulesetDiagnostic(
                GridPartSeverity::Error, "gravity_part_not_allowed_in_ruleset",
                "Gravity-dependent parts are not allowed by the requested ruleset.", &instance, profile.id));
        }
    }

    if (profile.requires_exit && !hasExit) {
        result.diagnostics.push_back(makeRulesetDiagnostic(GridPartSeverity::Error, "ruleset_requires_exit",
                                                           "Grid ruleset requires an exit objective signal.", nullptr,
                                                           profile.id));
    }
    if (profile.requires_player_spawn && !hasSpawn) {
        result.diagnostics.push_back(makeRulesetDiagnostic(GridPartSeverity::Error, "ruleset_requires_spawn",
                                                           "Grid ruleset requires a player spawn signal.", nullptr,
                                                           profile.id));
    }

    sortDiagnostics(result.diagnostics);
    result.ok = !result.hasErrors();
    return result;
}

} // namespace urpg::map
