param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$requiredPaths = @(
  "engine/core/quest/quest_registry.h",
  "engine/core/quest/quest_registry.cpp",
  "engine/core/quest/quest_validator.h",
  "engine/core/quest/quest_validator.cpp",
  "engine/core/dialogue/dialogue_graph.h",
  "engine/core/dialogue/dialogue_graph.cpp",
  "engine/core/narrative/narrative_continuity_checker.h",
  "engine/core/narrative/narrative_continuity_checker.cpp",
  "engine/core/narrative/choice_consequence_tracker.h",
  "engine/core/narrative/choice_consequence_tracker.cpp",
  "engine/core/narrative/ending_route_manager.h",
  "engine/core/narrative/ending_route_manager.cpp",
  "engine/core/relationship/relationship_registry.h",
  "engine/core/relationship/relationship_registry.cpp",
  "content/schemas/quest_registry.schema.json",
  "content/schemas/dialogue_graph.schema.json",
  "content/schemas/relationship_registry.schema.json",
  "content/fixtures/narrative_quest_fixture.json",
  "tests/unit/test_quest_registry.cpp",
  "tests/unit/test_dialogue_graph.cpp",
  "tests/unit/test_narrative_continuity_checker.cpp",
  "tests/unit/test_choice_consequence_tracker.cpp",
  "tests/unit/test_ending_route_manager.cpp",
  "tests/unit/test_relationship_registry.cpp"
)

$missing = @()
foreach ($path in $requiredPaths) {
  if (-not (Test-Path (Join-Path $RepoRoot $path))) {
    $missing += $path
  }
}

if ($missing.Count -gt 0) {
  throw "Missing FFS-10 narrative/quest files: $($missing -join ', ')"
}

$cmake = Get-Content (Join-Path $RepoRoot "CMakeLists.txt") -Raw
foreach ($needle in @(
  "engine/core/quest/quest_registry.cpp",
  "engine/core/dialogue/dialogue_graph.cpp",
  "engine/core/narrative/narrative_continuity_checker.cpp",
  "engine/core/relationship/relationship_registry.cpp",
  "tests/unit/test_quest_registry.cpp",
  "tests/unit/test_relationship_registry.cpp"
)) {
  if (-not $cmake.Contains($needle)) {
    throw "CMakeLists.txt does not register $needle"
  }
}

$plan = Get-Content (Join-Path $RepoRoot "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md") -Raw
if ($plan.Contains("- [ ] Quest registry supports objective states: locked, active, completed, failed, hidden.")) {
  throw "FFS-10 quest checklist remains unchecked."
}

Write-Host "FFS-10 narrative and quest governance checks passed."
