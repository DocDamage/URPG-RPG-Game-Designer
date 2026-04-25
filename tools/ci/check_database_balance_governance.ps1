param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$required = @(
  "content/schemas/rpg_database.schema.json",
  "content/schemas/balance_suite.schema.json",
  "content/fixtures/rpg_database_balance_fixture.json",
  "engine/core/database/rpg_database.h",
  "engine/core/balance/economy_simulator.h",
  "engine/core/balance/encounter_table.h",
  "engine/core/shop/vendor_catalog.h",
  "engine/core/rest/rest_point.h",
  "engine/core/items/loot_affix_generator.h",
  "engine/core/progression/class_progression.h",
  "engine/core/ability/skill_combo_rules.h",
  "tests/unit/test_rpg_database_suite.cpp",
  "tests/unit/test_economy_simulator.cpp",
  "tests/unit/test_encounter_table.cpp",
  "tests/unit/test_vendor_catalog.cpp",
  "tests/unit/test_rest_point.cpp",
  "tests/unit/test_loot_affix_generator.cpp",
  "tests/unit/test_class_progression.cpp",
  "tests/unit/test_skill_combo_rules.cpp"
)

foreach ($path in $required) {
  if (-not (Test-Path (Join-Path $RepoRoot $path))) {
    throw "Missing FFS-12 artifact: $path"
  }
}

$cmake = Get-Content -Raw -Path (Join-Path $RepoRoot "CMakeLists.txt")
foreach ($needle in @(
  "engine/core/database/rpg_database.cpp",
  "engine/core/balance/economy_simulator.cpp",
  "engine/core/shop/vendor_catalog.cpp",
  "editor/database/database_panel.cpp",
  "tests/unit/test_skill_combo_rules.cpp"
)) {
  if ($cmake -notmatch [regex]::Escape($needle)) {
    throw "CMake registration missing: $needle"
  }
}

Write-Host "FFS-12 database/balance governance artifacts are present."
