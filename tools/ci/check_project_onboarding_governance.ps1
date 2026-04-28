param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$requiredPaths = @(
  "engine/core/project/project_template_generator.h",
  "engine/core/project/project_template_generator.cpp",
  "engine/core/project/template_runtime_profile.h",
  "engine/core/project/template_runtime_profile.cpp",
  "engine/core/project/dev_room_generator.h",
  "engine/core/project/dev_room_generator.cpp",
  "engine/core/tutorial/tutorial_lesson.h",
  "engine/core/tutorial/tutorial_lesson.cpp",
  "editor/project/new_project_wizard_model.h",
  "editor/project/new_project_wizard_model.cpp",
  "editor/project/new_project_wizard_panel.h",
  "editor/project/new_project_wizard_panel.cpp",
  "content/templates/jrpg_starter.json",
  "content/templates/visual_novel_starter.json",
  "content/templates/turn_based_rpg_starter.json",
  "content/templates/tactics_rpg_starter.json",
  "content/templates/arpg_starter.json",
  "content/templates/monster_collector_rpg_starter.json",
  "content/templates/cozy_life_rpg_starter.json",
  "content/templates/metroidvania_lite_starter.json",
  "content/templates/2_5d_rpg_starter.json",
  "content/templates/roguelite_dungeon_starter.json",
  "content/templates/survival_horror_rpg_starter.json",
  "content/templates/farming_adventure_rpg_starter.json",
  "content/templates/card_battler_rpg_starter.json",
  "content/templates/platformer_rpg_starter.json",
  "content/templates/gacha_hero_rpg_starter.json",
  "content/templates/mystery_detective_rpg_starter.json",
  "content/templates/world_exploration_rpg_starter.json",
  "content/templates/space_opera_rpg_starter.json",
  "content/templates/post_apocalyptic_rpg_starter.json",
  "content/templates/tactical_mecha_rpg_starter.json",
  "content/templates/monster_tamer_arena_starter.json",
  "content/templates/soulslike_lite_rpg_starter.json",
  "content/templates/idle_incremental_rpg_starter.json",
  "content/templates/strategy_kingdom_rpg_starter.json",
  "content/templates/racing_adventure_rpg_starter.json",
  "content/templates/rhythm_rpg_starter.json",
  "content/templates/cooking_restaurant_rpg_starter.json",
  "content/templates/school_life_rpg_starter.json",
  "content/templates/pirate_rpg_starter.json",
  "content/templates/sports_team_rpg_starter.json",
  "content/templates/pet_shop_creature_care_rpg_starter.json",
  "content/templates/detective_noir_vn_rpg_starter.json",
  "content/templates/city_builder_rpg_starter.json",
  "content/templates/tower_defense_rpg_starter.json",
  "content/templates/beat_em_up_rpg_starter.json",
  "content/templates/open_world_survival_rpg_starter.json",
  "content/templates/faction_politics_rpg_starter.json",
  "content/fixtures/dev_room_fixture.json",
  "tests/unit/test_project_template_generator.cpp",
  "tests/unit/test_dev_room_generator.cpp",
  "tests/unit/test_tutorial_lesson.cpp"
)

$missing = @()
foreach ($path in $requiredPaths) {
  if (-not (Test-Path (Join-Path $RepoRoot $path))) {
    $missing += $path
  }
}

if ($missing.Count -gt 0) {
  throw "Missing FFS-08 onboarding files: $($missing -join ', ')"
}

$cmake = Get-Content (Join-Path $RepoRoot "CMakeLists.txt") -Raw
foreach ($needle in @(
  "engine/core/project/project_template_generator.cpp",
  "engine/core/project/template_runtime_profile.cpp",
  "engine/core/project/dev_room_generator.cpp",
  "engine/core/tutorial/tutorial_lesson.cpp",
  "editor/project/new_project_wizard_model.cpp",
  "editor/project/new_project_wizard_panel.cpp",
  "tests/unit/test_project_template_generator.cpp",
  "tests/unit/test_dev_room_generator.cpp",
  "tests/unit/test_tutorial_lesson.cpp"
)) {
  if (-not $cmake.Contains($needle)) {
    throw "CMakeLists.txt does not register $needle"
  }
}

$planPath = Join-Path $RepoRoot "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md"
if (-not (Test-Path $planPath)) {
  $planPath = Join-Path $RepoRoot "docs/archive/planning/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md"
}
$plan = Get-Content $planPath -Raw
if ($plan.Contains("- [ ] Generate project from template with maps, menu, message, battle, save, localization, input, and export profile.")) {
  throw "FFS-08 project-template checklist remains unchecked."
}

Write-Host "FFS-08 project onboarding governance checks passed."
