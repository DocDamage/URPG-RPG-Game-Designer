param(
    [string]$RepoRoot = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
} else {
    $RepoRoot = (Resolve-Path $RepoRoot).Path
}

function Require-File {
    param([string]$PathRel)
    $fullPath = Join-Path $RepoRoot $PathRel
    if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
        throw "Required map worldbuilding artifact missing: $PathRel"
    }
}

function Require-Text {
    param([string]$PathRel, [string]$Pattern, [string]$Message)
    $text = [System.IO.File]::ReadAllText((Join-Path $RepoRoot $PathRel))
    if ($text -notmatch $Pattern) {
        throw $Message
    }
}

$requiredFiles = @(
    "engine/core/map/tile_layer_document.h",
    "engine/core/map/terrain_brush.h",
    "engine/core/map/map_region_rules.h",
    "engine/core/map/procedural_map_generator.h",
    "engine/core/map/spawn_table.h",
    "engine/core/map/tactical_grid_preview.h",
    "engine/core/presentation/weather_profile.h",
    "editor/spatial/terrain_brush_panel.h",
    "editor/spatial/region_rules_panel.h",
    "editor/spatial/procedural_map_panel.h",
    "content/schemas/map_regions.schema.json",
    "content/schemas/procedural_map_profiles.schema.json",
    "content/fixtures/map_worldbuilding_fixture.json",
    "tests/unit/test_map_worldbuilding_suite.cpp"
)

foreach ($file in $requiredFiles) {
    Require-File -PathRel $file
}

Require-Text -PathRel "CMakeLists.txt" -Pattern "test_map_worldbuilding_suite\.cpp" -Message "Map worldbuilding tests are not registered in CMakeLists.txt."
Require-Text -PathRel "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md" -Pattern "FFS-06[\s\S]*implementation slice is complete" -Message "FFS-06 completion status is missing."

Write-Output "MAP_WORLDBUILDING_GOVERNANCE`tOK"
