param(
  [string]$RepoRoot = "."
)

$ErrorActionPreference = "Stop"

$requiredReports = @(
  "imports/reports/asset_hygiene_summary.json",
  "imports/reports/asset_hygiene_duplicates.csv",
  "imports/reports/asset_intake/source_capture_status.json",
  "imports/reports/asset_intake/wysiwyg_smoke_proof.json",
  "imports/reports/asset_intake/attribution/SRC-002_gdquest_blue_actor.json",
  "imports/reports/asset_intake/attribution/SRC-003_kenney_click_001.json"
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

$attributionRecords = @(
  "imports/reports/asset_intake/attribution/SRC-002_gdquest_blue_actor.json",
  "imports/reports/asset_intake/attribution/SRC-003_kenney_click_001.json"
)
foreach ($relativePath in $attributionRecords) {
  $path = Join-Path $RepoRoot $relativePath
  $record = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
  if ([string]::IsNullOrWhiteSpace($record.asset_id) -or
      [string]::IsNullOrWhiteSpace($record.source_id) -or
      [string]::IsNullOrWhiteSpace($record.original_relative_path) -or
      [string]::IsNullOrWhiteSpace($record.author) -or
      [string]::IsNullOrWhiteSpace($record.license)) {
    Write-Error "Asset attribution record is missing required identity or license fields: $relativePath"
    exit 1
  }
  if ($record.commercial_use_allowed -ne $true -or $record.redistribution_allowed -ne $true) {
    Write-Error "Asset attribution record is not release-safe for commercial redistribution: $relativePath"
    exit 1
  }
  $assetPath = Join-Path $RepoRoot $record.asset_id
  if (-not (Test-Path -LiteralPath $assetPath -PathType Leaf)) {
    Write-Error "Asset attribution record points at a missing promoted asset: $($record.asset_id)"
    exit 1
  }
}

Write-Host "Asset library governance reports are present and parseable."
