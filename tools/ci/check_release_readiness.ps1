$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$readinessPath = Join-Path $repoRoot "content\readiness\readiness_status.json"
$matrixPath = Join-Path $repoRoot "docs\RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs\TEMPLATE_READINESS_MATRIX.md"

if (-not (Test-Path $readinessPath)) {
    throw "Missing readiness dataset: $readinessPath"
}
if (-not (Test-Path $matrixPath)) {
    throw "Missing release readiness matrix: $matrixPath"
}
if (-not (Test-Path $templateMatrixPath)) {
    throw "Missing template readiness matrix: $templateMatrixPath"
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$tick = [string][char]96

if (-not $readiness.schemaVersion) {
    throw "Readiness dataset must include schemaVersion."
}

$allowedStatuses = @("READY", "PARTIAL", "EXPERIMENTAL", "BLOCKED", "PLANNED")

foreach ($entry in $readiness.subsystems) {
    if ($allowedStatuses -notcontains $entry.status) {
        throw "Subsystem '$($entry.id)' has invalid status '$($entry.status)'."
    }
}

foreach ($entry in $readiness.templates) {
    if ($allowedStatuses -notcontains $entry.status) {
        throw "Template '$($entry.id)' has invalid status '$($entry.status)'."
    }
}

$matrixText = Get-Content -Raw -Path $matrixPath
$templateMatrixText = Get-Content -Raw -Path $templateMatrixPath

foreach ($entry in $readiness.subsystems) {
    $pattern = [regex]::Escape($tick + $entry.id + $tick)
    if ($matrixText -notmatch $pattern) {
        throw "Release readiness matrix is missing subsystem row '$($entry.id)'."
    }
}

foreach ($entry in $readiness.templates) {
    $pattern = [regex]::Escape($tick + $entry.id + $tick)
    if ($templateMatrixText -notmatch $pattern) {
        throw "Template readiness matrix is missing template row '$($entry.id)'."
    }
}

Write-Host "Release readiness records and matrices are present and minimally aligned."
