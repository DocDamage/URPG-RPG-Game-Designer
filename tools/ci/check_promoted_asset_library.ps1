param(
  [string]$ReportPath = "imports/reports/asset_intake/promoted_asset_library_report.json"
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path)
$bundleRoot = Join-Path $repoRoot "imports/manifests/asset_bundles"
$normalizedRoot = Join-Path $repoRoot "imports/normalized"
$resolvedReportPath = if ([System.IO.Path]::IsPathRooted($ReportPath)) {
  [System.IO.Path]::GetFullPath($ReportPath)
} else {
  [System.IO.Path]::GetFullPath((Join-Path $repoRoot $ReportPath))
}

$errors = New-Object System.Collections.Generic.List[string]
$bundleReports = New-Object System.Collections.Generic.List[object]

function Add-Error {
  param([string]$Message)
  $script:errors.Add($Message) | Out-Null
}

$lfsPaths = New-Object System.Collections.Generic.HashSet[string]
@(git lfs ls-files --name-only | Where-Object { $_ -ne "" }) | ForEach-Object {
  $lfsPaths.Add(($_ -replace "\\", "/")) | Out-Null
}

$trackedPayloadPaths = New-Object System.Collections.Generic.HashSet[string]
@(git ls-files imports/normalized | Where-Object { $_ -ne "" }) | ForEach-Object {
  $trackedPayloadPaths.Add(($_ -replace "\\", "/")) | Out-Null
}

$totals = [ordered]@{
  bundle_count = 0
  candidate_manifest_count = 0
  asset_rows = 0
  promoted_rows = 0
  release_required_rows = 0
  present_payload_rows = 0
  local_untracked_payload_rows = 0
  lfs_tracked_payload_rows = 0
  permitted_history_only_rows = 0
}

foreach ($bundleFile in Get-ChildItem -LiteralPath $bundleRoot -Filter "BND-*.json" -File | Sort-Object Name) {
  try {
    $bundle = Get-Content -LiteralPath $bundleFile.FullName -Raw | ConvertFrom-Json
  } catch {
    Add-Error "Invalid JSON in bundle manifest: $($bundleFile.Name)"
    continue
  }

  if ($null -eq $bundle.PSObject.Properties["bundle_id"]) {
    $totals.candidate_manifest_count++
    $bundleReports.Add([ordered]@{
      file = $bundleFile.Name
      classification = "candidate_manifest_only"
      asset_rows = @($bundle.candidateAssets).Count
      notes = "Manifest has no bundle_id and is not treated as promoted payload."
    }) | Out-Null
    continue
  }

  $totals.bundle_count++
  $assets = @($bundle.assets)
  $bundleAssetRows = $assets.Count
  $bundlePromotedRows = 0
  $bundleReleaseRequiredRows = 0
  $bundlePresentPayloadRows = 0
  $bundleLocalUntrackedRows = 0
  $bundleLfsRows = 0
  $bundleHistoryOnlyRows = 0

  if ($bundle.bundle_state -ne "promoted") {
    Add-Error "Bundle $($bundle.bundle_id) is not promoted."
  }

  foreach ($asset in $assets) {
    $totals.asset_rows++
    if ($asset.status -eq "promoted") {
      $totals.promoted_rows++
      $bundlePromotedRows++
    } else {
      Add-Error "Bundle $($bundle.bundle_id) has non-promoted asset row: $($asset.promoted_relative_path)"
      continue
    }

    if ($asset.release_required -eq $true) {
      $totals.release_required_rows++
      $bundleReleaseRequiredRows++
    }

    foreach ($field in @("original_relative_path", "promoted_relative_path", "category", "distribution", "notes")) {
      if ($null -eq $asset.PSObject.Properties[$field] -or [string]::IsNullOrWhiteSpace([string]$asset.$field)) {
        Add-Error "Bundle $($bundle.bundle_id) promoted row is missing $field."
      }
    }

    $historyOnly = ($asset.distribution -eq "deferred" -and ([string]$asset.notes) -match "history only|future promoted audio")
    if ($asset.license_cleared -ne $true -and -not $historyOnly) {
      Add-Error "Bundle $($bundle.bundle_id) promoted row is not license-cleared: $($asset.promoted_relative_path)"
    }
    if ($asset.release_eligible -ne $true -and -not $historyOnly) {
      Add-Error "Bundle $($bundle.bundle_id) promoted row is not release-eligible: $($asset.promoted_relative_path)"
    }
    if ([string]::IsNullOrWhiteSpace([string]$asset.attribution_record) -and -not $historyOnly) {
      Add-Error "Bundle $($bundle.bundle_id) promoted row is missing attribution_record: $($asset.promoted_relative_path)"
    }
    if ([string]::IsNullOrWhiteSpace([string]$asset.checksum_sha256) -and -not $historyOnly) {
      Add-Error "Bundle $($bundle.bundle_id) promoted row is missing checksum_sha256: $($asset.promoted_relative_path)"
    }

    $relativePayloadPath = "imports/normalized/$($asset.promoted_relative_path)" -replace "\\", "/"
    $fullPayloadPath = Join-Path $normalizedRoot ([string]$asset.promoted_relative_path)
    if ($trackedPayloadPaths.Contains($relativePayloadPath)) {
      $totals.present_payload_rows++
      $bundlePresentPayloadRows++
      if ($lfsPaths.Contains($relativePayloadPath)) {
        $totals.lfs_tracked_payload_rows++
        $bundleLfsRows++
      }
    } elseif (Test-Path -LiteralPath $fullPayloadPath -PathType Leaf) {
      $totals.present_payload_rows++
      $bundlePresentPayloadRows++
      $totals.local_untracked_payload_rows++
      $bundleLocalUntrackedRows++
    } elseif ($historyOnly) {
      $totals.permitted_history_only_rows++
      $bundleHistoryOnlyRows++
    } else {
      Add-Error "Promoted payload is missing: $relativePayloadPath"
    }
  }

  $bundleReports.Add([ordered]@{
    file = $bundleFile.Name
    bundle_id = $bundle.bundle_id
    source_id = $bundle.source_id
    classification = "promoted_bundle"
    release_required = $bundle.release_required
    asset_rows = $bundleAssetRows
    promoted_rows = $bundlePromotedRows
    release_required_rows = $bundleReleaseRequiredRows
    present_payload_rows = $bundlePresentPayloadRows
    local_untracked_payload_rows = $bundleLocalUntrackedRows
    lfs_tracked_payload_rows = $bundleLfsRows
    permitted_history_only_rows = $bundleHistoryOnlyRows
  }) | Out-Null
}

$report = [ordered]@{
  schema = "urpg.promoted_asset_library_report.v1"
  generated_at_utc = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
  passed = ($errors.Count -eq 0)
  totals = $totals
  bundles = @($bundleReports.ToArray())
  errors = @($errors.ToArray())
}

$reportDir = Split-Path -Parent $resolvedReportPath
if (-not [string]::IsNullOrWhiteSpace($reportDir)) {
  New-Item -ItemType Directory -Force -Path $reportDir | Out-Null
}
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $resolvedReportPath -Encoding UTF8

if ($errors.Count -gt 0) {
  foreach ($errorItem in $errors) {
    Write-Host $errorItem
  }
  exit 1
}

Write-Host "Promoted asset library validation passed."
Write-Host "Promoted asset rows: $($totals.promoted_rows)"
Write-Host "Present payload rows: $($totals.present_payload_rows)"
Write-Host "Local untracked payload rows: $($totals.local_untracked_payload_rows)"
Write-Host "LFS-tracked promoted payload rows: $($totals.lfs_tracked_payload_rows)"
Write-Host "Promoted asset library report written: $resolvedReportPath"
