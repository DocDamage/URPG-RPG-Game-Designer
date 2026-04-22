$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\accessibility_report.schema.json",
    "engine\core\accessibility\accessibility_auditor.h",
    "engine\core\accessibility\accessibility_auditor.cpp",
    "editor\accessibility\accessibility_panel.h",
    "editor\accessibility\accessibility_panel.cpp",
    "editor\accessibility\accessibility_menu_adapter.h",
    "editor\accessibility\accessibility_menu_adapter.cpp",
    "tools\ci\check_accessibility_governance.ps1",
    "content\fixtures\accessibility_report_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required accessibility artifact: $relPath"
    }
}

# Validate schema is parseable JSON
$schemaPath = Join-Path $repoRoot "content\schemas\accessibility_report.schema.json"
if (Test-Path $schemaPath) {
    try {
        $null = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
    } catch {
        $errors += "Accessibility report schema is not valid JSON: $_"
    }
}

# Validate fixture is parseable JSON and matches schema shape minimally
$fixturePath = Join-Path $repoRoot "content\fixtures\accessibility_report_fixture.json"
if (Test-Path $fixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
        if (-not $fixture.auditDate) {
            $errors += "Accessibility report fixture is missing auditDate"
        }
        if (-not $fixture.issues) {
            $errors += "Accessibility report fixture is missing issues array"
        }
        foreach ($issue in $fixture.issues) {
            if (-not $issue.severity) {
                $errors += "Accessibility report fixture issue missing severity"
            }
            if (-not $issue.category) {
                $errors += "Accessibility report fixture issue missing category"
            }
            if (-not $issue.elementId) {
                $errors += "Accessibility report fixture issue missing elementId"
            }
            if (-not $issue.message) {
                $errors += "Accessibility report fixture issue missing message"
            }
        }
    } catch {
        $errors += "Accessibility report fixture is not valid JSON: $_"
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
