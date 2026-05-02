$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$readinessPath = Join-Path $repoRoot "content\readiness\readiness_status.json"
$matrixPath = Join-Path $repoRoot "docs\RELEASE_READINESS_MATRIX.md"

if (-not (Test-Path $readinessPath)) {
    throw "Missing readiness dataset: $readinessPath"
}
if (-not (Test-Path $matrixPath)) {
    throw "Missing release readiness matrix: $matrixPath"
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$matrixText = Get-Content -Raw -Path $matrixPath
$tick = [string][char]96

foreach ($entry in $readiness.subsystems) {
    $pattern = [regex]::Escape($tick + $entry.id + $tick)
    if ($matrixText -notmatch $pattern) {
        throw "Release readiness matrix is missing row for subsystem '$($entry.id)'."
    }

    $statusPattern = [regex]::Escape("| " + $tick + $entry.id + $tick + " | " + $tick + $entry.status + $tick + " |")
    if ($matrixText -notmatch $statusPattern) {
        throw "Release readiness matrix status for subsystem '$($entry.id)' does not match readiness_status.json '$($entry.status)'."
    }
}

Write-Host "Subsystem status claims are aligned with the current readiness dataset."
