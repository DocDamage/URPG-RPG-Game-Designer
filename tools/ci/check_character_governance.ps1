$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\character_identity.schema.json",
    "engine\core\character\character_identity.h",
    "engine\core\character\character_identity.cpp",
    "engine\core\character\character_identity_validator.h",
    "engine\core\character\character_identity_validator.cpp",
    "editor\character\character_creator_model.h",
    "editor\character\character_creator_model.cpp",
    "editor\character\character_creator_panel.h",
    "editor\character\character_creator_panel.cpp",
    "tools\ci\check_character_governance.ps1",
    "content\fixtures\character_identity_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required character artifact: $relPath"
    }
}

$schemaPath = Join-Path $repoRoot "content\schemas\character_identity.schema.json"
if (Test-Path $schemaPath) {
    try {
        $null = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
    } catch {
        $errors += "Character identity schema is not valid JSON: $_"
    }
}

$fixturePath = Join-Path $repoRoot "content\fixtures\character_identity_fixture.json"
if (Test-Path $fixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
        if (-not $fixture.schemaVersion) {
            $errors += "Character identity fixture is missing schemaVersion"
        }
        if (-not $fixture.name) {
            $errors += "Character identity fixture is missing name"
        }
        if (-not $fixture.classId) {
            $errors += "Character identity fixture is missing classId"
        }
        if ($fixture.appearanceTokens -and -not ($fixture.appearanceTokens -is [System.Array])) {
            $errors += "Character identity fixture appearanceTokens must be an array when present"
        }
    } catch {
        $errors += "Character identity fixture is not valid JSON: $_"
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
