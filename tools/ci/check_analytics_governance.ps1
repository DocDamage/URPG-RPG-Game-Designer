$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\analytics_config.schema.json",
    "engine\core\analytics\analytics_dispatcher.h",
    "engine\core\analytics\analytics_dispatcher.cpp",
    "engine\core\analytics\analytics_dispatcher_validator.h",
    "engine\core\analytics\analytics_dispatcher_validator.cpp",
    "editor\analytics\analytics_panel.h",
    "editor\analytics\analytics_panel.cpp",
    "tools\ci\check_analytics_governance.ps1",
    "content\fixtures\analytics_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required analytics artifact: $relPath"
    }
}

$schemaPath = Join-Path $repoRoot "content\schemas\analytics_config.schema.json"
if (Test-Path $schemaPath) {
    try {
        $null = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
    } catch {
        $errors += "Analytics config schema is not valid JSON: $_"
    }
}

$fixturePath = Join-Path $repoRoot "content\fixtures\analytics_fixture.json"
if (Test-Path $fixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
        if ($null -eq $fixture.optIn) {
            $errors += "Analytics fixture is missing optIn"
        }
        if (-not $fixture.sessionId) {
            $errors += "Analytics fixture is missing sessionId"
        }
        if ($fixture.allowedCategories -and -not ($fixture.allowedCategories -is [System.Array])) {
            $errors += "Analytics fixture allowedCategories must be an array when present"
        }
    } catch {
        $errors += "Analytics fixture is not valid JSON: $_"
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
