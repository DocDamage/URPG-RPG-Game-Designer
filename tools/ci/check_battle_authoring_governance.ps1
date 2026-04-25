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
        throw "Required battle authoring artifact missing: $PathRel"
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
    "engine/core/battle/battle_presentation_profile.h",
    "engine/core/battle/boss_profile.h",
    "engine/core/battle/battle_formula_probe.h",
    "engine/core/battle/enemy_ai_profile.h",
    "engine/core/battle/party_tactics_profile.h",
    "editor/battle/battle_presentation_panel.h",
    "editor/battle/boss_designer_panel.h",
    "editor/battle/formula_debugger_panel.h",
    "content/schemas/battle_presentation.schema.json",
    "content/schemas/boss_profiles.schema.json",
    "content/fixtures/battle_authoring_fixture.json",
    "tests/unit/test_battle_authoring_suite.cpp"
)

foreach ($file in $requiredFiles) {
    Require-File -PathRel $file
}

Require-Text -PathRel "CMakeLists.txt" -Pattern "test_battle_authoring_suite\.cpp" -Message "Battle authoring tests are not registered in CMakeLists.txt."
Require-Text -PathRel "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md" -Pattern "FFS-05[\s\S]*implementation slice is complete" -Message "FFS-05 completion status is missing."

Write-Output "BATTLE_AUTHORING_GOVERNANCE`tOK"
