$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$readinessPath = Join-Path $repoRoot "content\readiness\readiness_status.json"
$templateMatrixPath = Join-Path $repoRoot "docs\TEMPLATE_READINESS_MATRIX.md"

if (-not (Test-Path $readinessPath)) {
    throw "Missing readiness dataset: $readinessPath"
}
if (-not (Test-Path $templateMatrixPath)) {
    throw "Missing template matrix: $templateMatrixPath"
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$templateMatrixText = Get-Content -Raw -Path $templateMatrixPath
$tick = [string][char]96

foreach ($template in $readiness.templates) {
    $pattern = [regex]::Escape($tick + $template.id + $tick)
    if ($templateMatrixText -notmatch $pattern) {
        throw "Template matrix is missing row for '$($template.id)'."
    }
}

if ($templateMatrixText -match [regex]::Escape("| " + $tick + "arpg" + $tick + " | " + $tick + "READY" + $tick + " |")) {
    throw "ARPG must not be labeled READY in the template matrix."
}

Write-Host "Template claims are aligned with the current readiness dataset."
