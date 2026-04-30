param(
  [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path)
$manifestPath = Join-Path $repoRoot "content/fixtures/project_governance_fixture.json"
$bundleRoot = Join-Path $repoRoot "imports/manifests/asset_bundles"
$defaultReportPath = Join-Path $repoRoot "imports/reports/asset_intake/release_required_asset_report.json"
$requiredSurfaces = @("title", "map", "battle", "ui", "audio", "icons", "fonts")
$errors = New-Object System.Collections.Generic.List[string]
$connectedAssets = New-Object System.Collections.Generic.List[object]
$classifiedAssets = New-Object System.Collections.Generic.List[object]

function Add-Error {
  param([string]$Message)
  $script:errors.Add($Message) | Out-Null
}

function Test-LfsPointer {
  param([string]$Path)

  $reader = $null
  try {
    $reader = [System.IO.File]::OpenText($Path)
    return $reader.ReadLine() -eq "version https://git-lfs.github.com/spec/v1"
  } finally {
    if ($null -ne $reader) {
      $reader.Dispose()
    }
  }
}

function Resolve-RepoPath {
  param([string]$RelativePath)

  if ([System.IO.Path]::IsPathRooted($RelativePath)) {
    return [System.IO.Path]::GetFullPath($RelativePath)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $repoRoot $RelativePath))
}

$resolvedReportPath = if ([string]::IsNullOrWhiteSpace($ReportPath)) { $defaultReportPath } else { Resolve-RepoPath $ReportPath }

function Test-RepoPathUnderRoot {
  param([string]$Path)

  $fullPath = Resolve-RepoPath $Path
  return $fullPath.StartsWith($repoRoot, [System.StringComparison]::OrdinalIgnoreCase)
}

function Add-ConnectedAsset {
  param(
    [string]$Id,
    [string]$Surface,
    [string]$Source,
    [string]$Path,
    [string]$Classification,
    [string]$Notes
  )

  $script:connectedAssets.Add([ordered]@{
    id = $Id
    surface = $Surface
    source = $Source
    path = $Path
    classification = $Classification
    notes = $Notes
  }) | Out-Null
}

function Add-ClassifiedAsset {
  param(
    [string]$Id,
    [string]$Bundle,
    [string]$Path,
    [string]$Classification,
    [string]$Reason
  )

  $script:classifiedAssets.Add([ordered]@{
    id = $Id
    bundle = $Bundle
    path = $Path
    classification = $Classification
    reason = $Reason
  }) | Out-Null
}

function Write-Report {
  param([bool]$Passed)

  $reportDir = Split-Path -Parent $resolvedReportPath
  if (-not [string]::IsNullOrWhiteSpace($reportDir)) {
    New-Item -ItemType Directory -Force -Path $reportDir | Out-Null
  }

  $report = [ordered]@{
    schema = "urpg.release_required_asset_report.v1"
    generatedAt = (Get-Date).ToUniversalTime().ToString("o")
    passed = $Passed
    requiredSurfaces = @($requiredSurfaces)
    connectedAssets = @($connectedAssets.ToArray())
    classifiedAssets = @($classifiedAssets.ToArray())
    errors = @($errors.ToArray())
  }

  $report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $resolvedReportPath -Encoding UTF8
}

try {
  $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
} catch {
  Write-Host "Release required asset manifest is invalid JSON: $manifestPath"
  exit 1
}

if ($null -eq $manifest.releaseAssets) {
  Add-Error "Project governance fixture is missing releaseAssets."
} else {
  $releaseAssets = $manifest.releaseAssets
  if ($releaseAssets.manifestVersion -ne "1.0.0") {
    Add-Error "releaseAssets.manifestVersion must be 1.0.0."
  }
  if ($releaseAssets.assetBundleManifestRoot -ne "imports/manifests/asset_bundles") {
    Add-Error "releaseAssets.assetBundleManifestRoot must be imports/manifests/asset_bundles."
  }

  foreach ($surface in $requiredSurfaces) {
    if ($releaseAssets.requiredSurfaces -notcontains $surface) {
      Add-Error "releaseAssets.requiredSurfaces is missing '$surface'."
    }
    $surfaceAssets = @($releaseAssets.assets | Where-Object { $_.surface -eq $surface -and $_.required -eq $true })
    if ($surfaceAssets.Count -lt 1) {
      Add-Error "No required release asset entry covers surface '$surface'."
    }
  }

  foreach ($asset in @($releaseAssets.assets)) {
    if ($asset.required -ne $true) {
      Add-ClassifiedAsset -Id $asset.id -Bundle $asset.bundleId -Path $asset.path -Classification "deferred" -Reason "Project release asset entry is not marked required."
      continue
    }
    Add-ConnectedAsset -Id $asset.id -Surface $asset.surface -Source $asset.source -Path $asset.path -Classification "connected" -Notes $asset.notes
    if ($asset.licenseCleared -ne $true) {
      Add-Error "Required release asset '$($asset.id)' is not license-cleared."
    }
    if ($asset.source -eq "asset_bundle" -or $asset.source -eq "repo_resource") {
      if ([string]::IsNullOrWhiteSpace($asset.path)) {
        Add-Error "Required release asset '$($asset.id)' is missing path."
        continue
      }
      if (-not (Test-RepoPathUnderRoot $asset.path)) {
        Add-Error "Required release asset '$($asset.id)' resolves outside repo root: $($asset.path)"
        continue
      }
      if ($asset.path -match "^(third_party|vendor|imports/raw)/") {
        Add-Error "Required release asset '$($asset.id)' depends on raw/vendor content: $($asset.path)"
      }

      $fullPath = Resolve-RepoPath $asset.path
      if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
        Add-Error "Required release asset '$($asset.id)' is missing file: $($asset.path)"
        continue
      }
      if (Test-LfsPointer -Path $fullPath) {
        Add-Error "Required release asset '$($asset.id)' is an unresolved LFS pointer: $($asset.path)"
      }
      if ((Get-Item -LiteralPath $fullPath).Length -le 0) {
        Add-Error "Required release asset '$($asset.id)' is empty: $($asset.path)"
      }
    } elseif ($asset.source -eq "system_fallback") {
      if ($asset.distribution -ne "system_fallback") {
        Add-Error "System fallback asset '$($asset.id)' must use system_fallback distribution."
      }
    } else {
      Add-Error "Required release asset '$($asset.id)' has unsupported source '$($asset.source)'."
    }
  }
}

if (-not (Test-Path -LiteralPath $bundleRoot -PathType Container)) {
  Add-Error "Asset bundle manifest root is missing: imports/manifests/asset_bundles"
} else {
  foreach ($bundleFile in Get-ChildItem -LiteralPath $bundleRoot -Filter "BND-*.json" -File) {
    try {
      $bundle = Get-Content -LiteralPath $bundleFile.FullName -Raw | ConvertFrom-Json
    } catch {
      Add-Error "Asset bundle manifest is invalid JSON: $($bundleFile.Name)"
      continue
    }

    $releaseAssets = @($bundle.assets | Where-Object { $_.release_required -eq $true })
    if ($bundle.release_required -eq $true -and $releaseAssets.Count -lt 1) {
      Add-Error "Release-required bundle has no release-required asset rows: $($bundleFile.Name)"
    }

    foreach ($asset in @($bundle.assets | Where-Object { $_.release_required -ne $true })) {
      $classification = if ($asset.distribution -eq "source_only") { "source_only" } else { "deferred" }
      $reason = if ($classification -eq "source_only") {
        "Bundle asset is retained only as source/provenance material."
      } else {
        "Bundle asset is not connected to release-required surfaces."
      }
      Add-ClassifiedAsset -Id $asset.original_relative_path -Bundle $bundle.bundle_id -Path $asset.promoted_relative_path -Classification $classification -Reason $reason
    }

    foreach ($asset in $releaseAssets) {
      Add-ConnectedAsset -Id $asset.original_relative_path -Surface ($asset.release_surfaces -join ",") -Source "asset_bundle" -Path ("imports/normalized/" + $asset.promoted_relative_path) -Classification "connected" -Notes $asset.notes
      if ($bundle.bundle_state -ne "promoted" -or $asset.status -ne "promoted") {
        Add-Error "Release-required bundle asset must be promoted: $($bundleFile.Name) / $($asset.promoted_relative_path)"
      }
      if ($asset.license_cleared -ne $true) {
        Add-Error "Release-required bundle asset is not license-cleared: $($bundleFile.Name) / $($asset.promoted_relative_path)"
      }
      if ($asset.distribution -ne "bundled") {
        Add-Error "Release-required bundle asset must use bundled distribution: $($bundleFile.Name) / $($asset.promoted_relative_path)"
      }
      foreach ($surface in @($asset.release_surfaces)) {
        if ($requiredSurfaces -notcontains $surface) {
          Add-Error "Release-required bundle asset has unknown surface '$surface': $($bundleFile.Name)"
        }
      }

      $relativePath = $asset.promoted_relative_path
      if ([string]::IsNullOrWhiteSpace($relativePath)) {
        Add-Error "Release-required bundle asset is missing promoted_relative_path: $($bundleFile.Name)"
        continue
      }
      $normalizedPath = Resolve-RepoPath ("imports/normalized/" + $relativePath)
      if (-not $normalizedPath.StartsWith((Join-Path $repoRoot "imports\normalized"), [System.StringComparison]::OrdinalIgnoreCase)) {
        Add-Error "Release-required bundle asset escapes imports/normalized: $($bundleFile.Name) / $relativePath"
        continue
      }
      if (-not (Test-Path -LiteralPath $normalizedPath -PathType Leaf)) {
        Add-Error "Release-required bundle asset file is missing: imports/normalized/$relativePath"
        continue
      }
      if (Test-LfsPointer -Path $normalizedPath) {
        Add-Error "Release-required bundle asset is an unresolved LFS pointer: imports/normalized/$relativePath"
      }
    }
  }
}

if ($errors.Count -gt 0) {
  Write-Report -Passed $false
  foreach ($errorItem in $errors) {
    Write-Host $errorItem
  }
  exit 1
}

Write-Report -Passed $true
Write-Host "Release-required asset validation passed."
Write-Host "Release-required asset report written: $resolvedReportPath"
