$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\mod_manifest.schema.json",
    "engine\core\mod\mod_registry.h",
    "engine\core\mod\mod_registry.cpp",
    "engine\core\mod\mod_registry_validator.h",
    "engine\core\mod\mod_registry_validator.cpp",
    "editor\mod\mod_manager_panel.h",
    "editor\mod\mod_manager_panel.cpp",
    "tools\ci\check_mod_governance.ps1",
    "content\fixtures\mod_manifest_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required mod artifact: $relPath"
    }
}

$schemaPath = Join-Path $repoRoot "content\schemas\mod_manifest.schema.json"
if (Test-Path $schemaPath) {
    try {
        $null = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
    } catch {
        $errors += "Mod manifest schema is not valid JSON: $_"
    }
}

$fixturePath = Join-Path $repoRoot "content\fixtures\mod_manifest_fixture.json"
if (Test-Path $fixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
        if (-not $fixture.id) {
            $errors += "Mod manifest fixture is missing id"
        }
        if (-not $fixture.name) {
            $errors += "Mod manifest fixture is missing name"
        }
        if (-not $fixture.version) {
            $errors += "Mod manifest fixture is missing version"
        }
        if ($fixture.dependencies -and -not ($fixture.dependencies -is [System.Array])) {
            $errors += "Mod manifest fixture dependencies must be an array when present"
        }
    } catch {
        $errors += "Mod manifest fixture is not valid JSON: $_"
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
