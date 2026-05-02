$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\audio_mix_presets.schema.json",
    "engine\core\audio\audio_mix_presets.h",
    "engine\core\audio\audio_mix_presets.cpp",
    "engine\core\audio\audio_mix_validator.h",
    "engine\core\audio\audio_mix_validator.cpp",
    "engine\core\audio\audio_mix_backend_smoke.h",
    "engine\core\audio\audio_mix_backend_smoke.cpp",
    "editor\audio\audio_mix_panel.h",
    "editor\audio\audio_mix_panel.cpp",
    "tools\ci\check_audio_governance.ps1",
    "content\fixtures\audio_mix_presets_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required audio artifact: $relPath"
    }
}

# Validate schema is parseable JSON
$schemaPath = Join-Path $repoRoot "content\schemas\audio_mix_presets.schema.json"
if (Test-Path $schemaPath) {
    try {
        $null = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
    } catch {
        $errors += "Audio mix presets schema is not valid JSON: $_"
    }
}

# Validate fixture is parseable JSON and matches schema shape minimally
$fixturePath = Join-Path $repoRoot "content\fixtures\audio_mix_presets_fixture.json"
if (Test-Path $fixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
        if (-not $fixture.version) {
            $errors += "Audio mix fixture is missing version"
        }
        if (-not $fixture.presets) {
            $errors += "Audio mix fixture is missing presets array"
        }
        $hasDefault = $false
        foreach ($preset in $fixture.presets) {
            if (-not $preset.name) {
                $errors += "Audio mix fixture preset missing name"
            }
            if ($preset.name -eq "Default") {
                $hasDefault = $true
            }
            if (-not $preset.categoryVolumes) {
                $errors += "Audio mix fixture preset '$($preset.name)' missing categoryVolumes"
            }
            if ($null -eq $preset.duckBGMOnSE) {
                $errors += "Audio mix fixture preset '$($preset.name)' missing duckBGMOnSE"
            }
            if ($null -eq $preset.duckAmount) {
                $errors += "Audio mix fixture preset '$($preset.name)' missing duckAmount"
            }
        }
        if (-not $hasDefault) {
            $errors += "Audio mix fixture is missing a 'Default' preset"
        }
    } catch {
        $errors += "Audio mix fixture is not valid JSON: $_"
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
