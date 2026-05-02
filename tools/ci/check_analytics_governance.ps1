$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\analytics_config.schema.json",
    "content\schemas\analytics_endpoint_profile.schema.json",
    "engine\core\analytics\analytics_dispatcher.h",
    "engine\core\analytics\analytics_dispatcher.cpp",
    "engine\core\analytics\analytics_endpoint_profile.h",
    "engine\core\analytics\analytics_endpoint_profile.cpp",
    "engine\core\analytics\analytics_dispatcher_validator.h",
    "engine\core\analytics\analytics_dispatcher_validator.cpp",
    "editor\analytics\analytics_panel.h",
    "editor\analytics\analytics_panel.cpp",
    "tools\ci\check_analytics_governance.ps1",
    "content\fixtures\analytics_fixture.json",
    "content\fixtures\analytics_endpoint_profile_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required analytics artifact: $relPath"
    }
}

$endpointFixturePath = Join-Path $repoRoot "content\fixtures\analytics_endpoint_profile_fixture.json"
if (Test-Path $endpointFixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $endpointFixturePath | ConvertFrom-Json
        if ($fixture.schema -ne "urpg.analytics_endpoint_profile.v1") {
            $errors += "Analytics endpoint profile fixture has unexpected schema"
        }
        if (-not $fixture.profileId) {
            $errors += "Analytics endpoint profile fixture is missing profileId"
        }
        if ($fixture.mode -eq "http_json") {
            if (-not $fixture.url) {
                $errors += "HTTP analytics endpoint profile fixture is missing url"
            }
            if ($null -eq $fixture.privacyReview -or $fixture.privacyReview.approved -ne $true) {
                $errors += "HTTP analytics endpoint profile fixture requires approved privacyReview"
            }
            if ($null -eq $fixture.privacyReview.dataCategories -or $fixture.privacyReview.dataCategories.Count -eq 0) {
                $errors += "HTTP analytics endpoint profile fixture requires privacyReview dataCategories"
            }
        }
    } catch {
        $errors += "Analytics endpoint profile fixture is not valid JSON: $_"
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

$endpointSchemaPath = Join-Path $repoRoot "content\schemas\analytics_endpoint_profile.schema.json"
if (Test-Path $endpointSchemaPath) {
    try {
        $null = Get-Content -Raw -Path $endpointSchemaPath | ConvertFrom-Json
    } catch {
        $errors += "Analytics endpoint profile schema is not valid JSON: $_"
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
