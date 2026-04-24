param(
  [string]$RepoRoot = "."
)

$ErrorActionPreference = "Stop"

$requiredReports = @(
  "imports/reports/asset_hygiene_summary.json",
  "imports/reports/asset_hygiene_duplicates.csv",
  "imports/reports/asset_intake/source_capture_status.json",
  "imports/reports/asset_intake/wysiwyg_smoke_proof.json"
)

$missing = @()
foreach ($relativePath in $requiredReports) {
  $path = Join-Path $RepoRoot $relativePath
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
    $missing += $relativePath
  }
}

if ($missing.Count -gt 0) {
  Write-Error ("Asset library governance missing required report(s): " + ($missing -join ", "))
  exit 1
}

$summaryPath = Join-Path $RepoRoot "imports/reports/asset_hygiene_summary.json"
$summary = Get-Content -LiteralPath $summaryPath -Raw | ConvertFrom-Json
if ($null -eq $summary.file_count -or $null -eq $summary.duplicate_groups -or $null -eq $summary.oversize_count) {
  Write-Error "Asset hygiene summary is missing file_count, duplicate_groups, or oversize_count."
  exit 1
}

$intakePath = Join-Path $RepoRoot "imports/reports/asset_intake/source_capture_status.json"
$intake = Get-Content -LiteralPath $intakePath -Raw | ConvertFrom-Json
if ($null -eq $intake.sources -or $intake.sources.Count -eq 0) {
  Write-Error "Asset intake source capture report has no sources."
  exit 1
}

Write-Host "Asset library governance reports are present and parseable."
