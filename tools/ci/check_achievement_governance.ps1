$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\achievements.schema.json",
    "engine\core\achievement\achievement_registry.h",
    "engine\core\achievement\achievement_registry.cpp",
    "engine\core\achievement\achievement_validator.h",
    "engine\core\achievement\achievement_validator.cpp",
    "editor\achievement\achievement_panel.h",
    "editor\achievement\achievement_panel.cpp",
    "tools\ci\check_achievement_governance.ps1",
    "content\fixtures\achievement_registry_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required achievement artifact: $relPath"
    }
}

# Validate schema is parseable JSON
$schemaPath = Join-Path $repoRoot "content\schemas\achievements.schema.json"
if (Test-Path $schemaPath) {
    try {
        $null = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
    } catch {
        $errors += "Achievement schema is not valid JSON: $_"
    }
}

# Validate fixture is parseable JSON and matches schema shape minimally
$fixturePath = Join-Path $repoRoot "content\fixtures\achievement_registry_fixture.json"
if (Test-Path $fixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
        if (-not $fixture.version) {
            $errors += "Achievement fixture is missing version"
        }
        if (-not $fixture.progress) {
            $errors += "Achievement fixture is missing progress array"
        }
        foreach ($entry in $fixture.progress) {
            if (-not $entry.id) {
                $errors += "Achievement fixture progress entry missing id"
            }
            if ($null -eq $entry.current) {
                $errors += "Achievement fixture progress entry '$($entry.id)' missing current"
            }
            if ($null -eq $entry.target) {
                $errors += "Achievement fixture progress entry '$($entry.id)' missing target"
            }
            if ($null -eq $entry.unlocked) {
                $errors += "Achievement fixture progress entry '$($entry.id)' missing unlocked"
            }
        }
    } catch {
        $errors += "Achievement fixture is not valid JSON: $_"
    }
}

$result = @{
    passed = $errors.Count -eq 0
    errors = $errors
}

$result | ConvertTo-Json -Depth 3

if ($errors.Count -gt 0) {
    exit 1
}
