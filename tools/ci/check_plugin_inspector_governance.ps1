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
        throw "Required plugin inspector artifact missing: $PathRel"
    }
}

function Require-Text {
    param(
        [string]$PathRel,
        [string]$Pattern,
        [string]$Message
    )
    $fullPath = Join-Path $RepoRoot $PathRel
    $text = [System.IO.File]::ReadAllText($fullPath)
    if ($text -notmatch $Pattern) {
        throw $Message
    }
}

$requiredFiles = @(
    "engine/core/plugin/plugin_compatibility_score.h",
    "engine/core/plugin/plugin_compatibility_score.cpp",
    "editor/plugin/plugin_inspector_model.h",
    "editor/plugin/plugin_inspector_model.cpp",
    "editor/plugin/plugin_inspector_panel.h",
    "editor/plugin/plugin_inspector_panel.cpp",
    "tests/unit/test_plugin_compatibility_score.cpp",
    "tests/unit/test_plugin_inspector_panel.cpp",
    "content/schemas/plugin_manifest.schema.json",
    "tests/compat/fixtures/plugins/URPG_DependencyDrift_Fixture.json"
)

foreach ($file in $requiredFiles) {
    Require-File -PathRel $file
}

Require-Text `
    -PathRel "engine/core/plugin/plugin_compatibility_score.cpp" `
    -Pattern "release_authoritative.*false" `
    -Message "Plugin compatibility report must remain non-release-authoritative."

Require-Text `
    -PathRel "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md" `
    -Pattern "FFS-04[\s\S]*not release-authoritative" `
    -Message "FFS-04 docs must explicitly avoid release-authoritative claims."

Require-Text `
    -PathRel "CMakeLists.txt" `
    -Pattern "test_plugin_compatibility_score\.cpp" `
    -Message "FFS-04 scorer tests are not registered in CMakeLists.txt."

Require-Text `
    -PathRel "CMakeLists.txt" `
    -Pattern "test_plugin_inspector_panel\.cpp" `
    -Message "FFS-04 inspector panel tests are not registered in CMakeLists.txt."

Write-Output "PLUGIN_INSPECTOR_GOVERNANCE`tOK"
