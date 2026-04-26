[CmdletBinding()]
param(
  [ValidateSet("DevUnsigned", "Release")]
  [string]$Mode = "DevUnsigned",

  [string]$BuildDirectory = "build/dev-ninja-debug",

  [string]$OutputRoot = "build/release-packages",

  [string]$ReportPath,

  [switch]$DryRun,

  [switch]$Json
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")

function ConvertTo-RepoPath {
  param([Parameter(Mandatory = $true)][string]$Path)

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }

  return [System.IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
}

function Get-RequiredSigningInputs {
  return @(
    [ordered]@{
      target = "Windows_x64"
      step = "sign"
      tool = "signtool"
      requiredEnvironment = @("URPG_WINDOWS_SIGN_CERT_PATH", "URPG_WINDOWS_SIGN_CERT_PASSWORD")
      notarization = "not_applicable"
    },
    [ordered]@{
      target = "Linux_x64"
      step = "sign"
      tool = "gpg"
      requiredEnvironment = @("URPG_LINUX_SIGNING_KEY_PATH")
      notarization = "not_applicable"
    },
    [ordered]@{
      target = "macOS_Universal"
      step = "sign_and_notarize"
      tool = "codesign/notarytool"
      requiredEnvironment = @("URPG_MACOS_DEVELOPER_ID_APPLICATION", "URPG_MACOS_NOTARY_PROFILE")
      notarization = "required"
    },
    [ordered]@{
      target = "Web_WASM"
      step = "manifest_integrity_only"
      tool = "bundle_signature"
      requiredEnvironment = @()
      notarization = "not_applicable"
    }
  )
}

function Test-SigningInputs {
  param([Parameter(Mandatory = $true)][array]$Inputs)

  $missing = @()
  foreach ($targetInput in $Inputs) {
    foreach ($name in $targetInput.requiredEnvironment) {
      if ([string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($name))) {
        $missing += [ordered]@{
          target = $targetInput.target
          variable = $name
        }
      }
    }
  }
  return @($missing)
}

function New-BaseReport {
  param(
    [Parameter(Mandatory = $true)][string]$Mode,
    [Parameter(Mandatory = $true)][string]$BuildDirectory,
    [Parameter(Mandatory = $true)][string]$OutputRoot,
    [Parameter(Mandatory = $true)][bool]$DryRun
  )

  $signingInputs = Get-RequiredSigningInputs
  $pluginDropinsManifest = ConvertTo-RepoPath "third_party/rpgmaker-mz/steam-dlc/reports/plugin_dropins_release_manifest.json"
  $pluginDropinsReleaseRoot = ConvertTo-RepoPath "third_party/rpgmaker-mz/steam-dlc/plugin-dropins-curated/js/plugins"
  return [ordered]@{
    tool = "package_release_artifacts"
    mode = $Mode
    dryRun = $DryRun
    success = $false
    buildDirectory = $BuildDirectory
    outputRoot = $OutputRoot
    unsignedArtifactsAllowed = $Mode -eq "DevUnsigned"
    signingInputs = @($signingInputs)
    missingSigningInputs = @()
    pluginDropinsManifest = $pluginDropinsManifest
    pluginDropinsReleaseRoot = $pluginDropinsReleaseRoot
    pluginDropinsManifestPresent = Test-Path -LiteralPath $pluginDropinsManifest
    exportMatrix = $null
    artifacts = @()
    releaseModePolicy = if ($Mode -eq "Release") {
      "fail_without_signing_and_notarization_credentials"
    } else {
      "unsigned_development_artifacts_must_be_reported"
    }
  }
}

function Write-Report {
  param(
    [Parameter(Mandatory = $true)]$Report,
    [string]$ReportPath,
    [switch]$Json
  )

  $jsonText = $Report | ConvertTo-Json -Depth 64
  if (-not [string]::IsNullOrWhiteSpace($ReportPath)) {
    $resolvedReportPath = ConvertTo-RepoPath $ReportPath
    New-Item -ItemType Directory -Path (Split-Path $resolvedReportPath -Parent) -Force | Out-Null
    Set-Content -LiteralPath $resolvedReportPath -Value $jsonText -Encoding UTF8
  }

  if ($Json) {
    Write-Output $jsonText
  } else {
    Write-Host "Release package mode: $($Report.mode)"
    Write-Host "Release package dry run: $($Report.dryRun)"
    Write-Host "Release package success: $($Report.success)"
    if ($Report.missingSigningInputs.Count -gt 0) {
      foreach ($missing in $Report.missingSigningInputs) {
        Write-Host "Missing signing input for $($missing.target): $($missing.variable)" -ForegroundColor Red
      }
    }
  }
}

function Invoke-ExportMatrix {
  param(
    [Parameter(Mandatory = $true)][string]$BuildDirectory,
    [Parameter(Mandatory = $true)][string]$OutputRoot
  )

  $matrixJson = Join-Path ([System.IO.Path]::GetTempPath()) ("urpg_release_export_matrix_{0}.json" -f ([guid]::NewGuid()))
  try {
    & "$PSScriptRoot\check_platform_exports.ps1" -BuildDirectory $BuildDirectory -ExportDir (Join-Path $OutputRoot "exports") -Json > $matrixJson
    $exitCode = $LASTEXITCODE
    $matrixText = if (Test-Path $matrixJson) { Get-Content -Raw $matrixJson } else { "" }
    $matrixReport = if (-not [string]::IsNullOrWhiteSpace($matrixText)) { $matrixText | ConvertFrom-Json } else { $null }
    return [pscustomobject]@{
      exitCode = $exitCode
      report = $matrixReport
    }
  } finally {
    Remove-Item -LiteralPath $matrixJson -Force -ErrorAction SilentlyContinue
  }
}

$resolvedBuildDirectory = ConvertTo-RepoPath $BuildDirectory
$resolvedOutputRoot = ConvertTo-RepoPath $OutputRoot
$report = New-BaseReport -Mode $Mode -BuildDirectory $resolvedBuildDirectory -OutputRoot $resolvedOutputRoot -DryRun ([bool]$DryRun)

if (-not $report.pluginDropinsManifestPresent) {
  $report.success = $false
  Write-Report -Report $report -ReportPath $ReportPath -Json:$Json
  exit 4
}

if ($Mode -eq "Release") {
  $missingSigningInputs = Test-SigningInputs -Inputs $report.signingInputs
  $report.missingSigningInputs = @($missingSigningInputs)
  if ($missingSigningInputs.Count -gt 0) {
    $report.success = $false
    Write-Report -Report $report -ReportPath $ReportPath -Json:$Json
    exit 2
  }
}

if (-not $DryRun) {
  $matrix = Invoke-ExportMatrix -BuildDirectory $resolvedBuildDirectory -OutputRoot $resolvedOutputRoot
  $report.exportMatrix = $matrix.report
  if ($matrix.exitCode -ne 0 -or $null -eq $matrix.report -or $matrix.report.success -ne $true) {
    $report.success = $false
    Write-Report -Report $report -ReportPath $ReportPath -Json:$Json
    exit 3
  }

  foreach ($result in $matrix.report.results) {
    if ($result.status -eq "passed") {
      $report.artifacts += [ordered]@{
        target = $result.target
        outputDir = $result.outputDir
        signed = $Mode -eq "Release"
        notarized = $Mode -eq "Release" -and $result.target -eq "macOS_Universal"
        unsignedReason = if ($Mode -eq "DevUnsigned") { "explicit_dev_unsigned_mode" } else { $null }
      }
    }
  }
} else {
  foreach ($input in $report.signingInputs) {
    $report.artifacts += [ordered]@{
      target = $input.target
      outputDir = Join-Path $resolvedOutputRoot ("exports\" + $input.target)
      signed = $Mode -eq "Release"
      notarized = $Mode -eq "Release" -and $input.target -eq "macOS_Universal"
      unsignedReason = if ($Mode -eq "DevUnsigned") { "explicit_dev_unsigned_dry_run" } else { $null }
    }
  }
}

$report.success = $true
Write-Report -Report $report -ReportPath $ReportPath -Json:$Json
