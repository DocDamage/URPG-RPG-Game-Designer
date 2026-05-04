param(
  [string]$ConfigurePreset = "dev-ninja-release",
  [string]$BuildPreset = "dev-release",
  [string]$BuildDirectory = "build/dev-ninja-release",
  [string]$Configuration = "",
  [string]$InstallPrefix = "build/release-candidate-install",
  [string]$PackageRoot = "build/release-candidate-package",
  [int]$CTestTimeoutSeconds = 300,
  [switch]$SkipConfigure,
  [switch]$SkipBuild,
  [switch]$SkipLfsHydration,
  [string]$LfsWaiverReference = ""
)

$ErrorActionPreference = "Stop"

function Resolve-UnderRoot {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Root,
    [Parameter(Mandatory = $true)]
    [string]$Path
  )

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }

  return [System.IO.Path]::GetFullPath((Join-Path $Root $Path))
}

function Assert-LastExitCode {
  param(
    [Parameter(Mandatory = $true)]
    [string]$StepName
  )

  if ($LASTEXITCODE -ne 0) {
    throw "$StepName failed with exit code $LASTEXITCODE."
  }
}

function Invoke-GateStep {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Name,
    [Parameter(Mandatory = $true)]
    [scriptblock]$Script
  )

  Write-Host "== $Name ==" -ForegroundColor Cyan
  & $Script
}

function Test-RequiredDocs {
  param(
    [Parameter(Mandatory = $true)]
    [string]$RepoRoot
  )

  $requiredDocs = @(
    "README.md",
    "LICENSE",
    "CHANGELOG.md",
    "THIRD_PARTY_NOTICES.md",
    "EULA.md",
    "PRIVACY_POLICY.md",
    "CREDITS.md",
    "docs/APP_RELEASE_READINESS_MATRIX.md",
    "docs/release/AAA_RELEASE_READINESS_REPORT.md",
    "docs/release/RELEASE_PACKAGING.md",
    "docs/release/RELEASE_READINESS_MATRIX.md",
    "docs/status/PROGRAM_COMPLETION_STATUS.md"
  )

  $missing = @()
  foreach ($relativePath in $requiredDocs) {
    if (-not (Test-Path -LiteralPath (Join-Path $RepoRoot $relativePath) -PathType Leaf)) {
      $missing += $relativePath
    }
  }

  if ($missing.Count -gt 0) {
    throw "Release candidate gate is missing required document(s): $($missing -join ', ')"
  }
}

function Test-LfsPointerFile {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Path
  )

  $reader = $null
  try {
    $reader = [System.IO.File]::OpenText($Path)
    $firstLine = $reader.ReadLine()
    return $firstLine -eq "version https://git-lfs.github.com/spec/v1"
  } finally {
    if ($null -ne $reader) {
      $reader.Dispose()
    }
  }
}

function Assert-ReleaseLfsFileHydrated {
  param(
    [Parameter(Mandatory = $true)]
    [string]$ClonePath,
    [Parameter(Mandatory = $true)]
    [string]$RelativePath
  )

  $fullPath = Join-Path $ClonePath $RelativePath
  if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
    throw "Required release LFS file is missing from fresh clone: $RelativePath"
  }

  if (Test-LfsPointerFile -Path $fullPath) {
    throw "Required release LFS file is still an unresolved pointer after hydration: $RelativePath"
  }

  $stream = $null
  try {
    $stream = [System.IO.File]::OpenRead($fullPath)
    if ($stream.Length -le 0) {
      throw "Required release LFS file is empty after hydration: $RelativePath"
    }
  } finally {
    if ($null -ne $stream) {
      $stream.Dispose()
    }
  }
}

function Invoke-LfsHydrationCheck {
  param(
    [Parameter(Mandatory = $true)]
    [string]$RepoRoot
  )

  if ($SkipLfsHydration) {
    if ([string]::IsNullOrWhiteSpace($LfsWaiverReference)) {
      throw "LFS hydration skip requested without -LfsWaiverReference. Reference the active waiver/blocker record explicitly."
    }

    Write-Warning "Skipping fresh-clone LFS hydration because of waiver/blocker reference: $LfsWaiverReference"
    return
  }

  $remote = (& git -C $RepoRoot remote get-url origin).Trim()
  Assert-LastExitCode "Resolve git remote"

  $branch = (& git -C $RepoRoot rev-parse --abbrev-ref HEAD).Trim()
  Assert-LastExitCode "Resolve current branch"
  if ([string]::IsNullOrWhiteSpace($branch) -or $branch -eq "HEAD") {
    $branch = "development"
  }

  $tempRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath())
  $clonePath = Join-Path $tempRoot ("urpg-rc-lfs-{0:yyyyMMdd-HHmmss}" -f (Get-Date))
  Write-Host "Fresh clone path: $clonePath"

  $releaseRequiredLfsPaths = @(
    "resources/icons/urpg_editor.png",
    "resources/icons/urpg_runtime.png"
  )
  $releaseLfsInclude = $releaseRequiredLfsPaths -join ","

  $previousSkipSmudge = $env:GIT_LFS_SKIP_SMUDGE
  $env:GIT_LFS_SKIP_SMUDGE = "1"
  try {
    git clone --depth 1 --branch $branch --filter=blob:none $remote $clonePath
    Assert-LastExitCode "Fresh clone for LFS hydration"
  } finally {
    if ($null -eq $previousSkipSmudge) {
      Remove-Item Env:GIT_LFS_SKIP_SMUDGE -ErrorAction SilentlyContinue
    } else {
      $env:GIT_LFS_SKIP_SMUDGE = $previousSkipSmudge
    }
  }

  try {
    git -C $clonePath lfs install --local
    Assert-LastExitCode "Initialize Git LFS in fresh clone"

    git -C $clonePath lfs pull --include="$releaseLfsInclude" --exclude=""
    Assert-LastExitCode "Fresh-clone release-required Git LFS pull"

    foreach ($relativePath in $releaseRequiredLfsPaths) {
      Assert-ReleaseLfsFileHydrated -ClonePath $clonePath -RelativePath $relativePath
    }

    git -C $clonePath lfs fsck --pointers
    Assert-LastExitCode "Fresh-clone Git LFS pointer fsck"
  } finally {
    if (Test-Path -LiteralPath $clonePath) {
      $resolvedClone = [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $clonePath).Path)
      if (-not $resolvedClone.StartsWith($tempRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove LFS smoke path outside temp: $resolvedClone"
      }
      Remove-Item -LiteralPath $resolvedClone -Recurse -Force
    }
  }
}

$repoRoot = [System.IO.Path]::GetFullPath((Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path)
$buildPath = Resolve-UnderRoot -Root $repoRoot -Path $BuildDirectory
$installPath = Resolve-UnderRoot -Root $repoRoot -Path $InstallPrefix
$packagePath = Resolve-UnderRoot -Root $repoRoot -Path $PackageRoot

$script:RcConfigurePreset = $ConfigurePreset
$script:RcBuildPreset = $BuildPreset
$script:RcBuildDirectory = $BuildDirectory
$script:RcConfiguration = $Configuration
$script:RcCTestTimeoutSeconds = $CTestTimeoutSeconds
$script:RcBuildPath = $buildPath
$script:RcInstallPath = $installPath
$script:RcPackagePath = $packagePath
$script:RcRepoRoot = $repoRoot

Push-Location $repoRoot
try {
  Invoke-GateStep "Check required release documents" {
    Test-RequiredDocs -RepoRoot $repoRoot
  }

  Invoke-GateStep "Check fresh-clone LFS hydration" {
    Invoke-LfsHydrationCheck -RepoRoot $repoRoot
  }

  Invoke-GateStep "Check release-required asset manifest" {
    & "$PSScriptRoot\check_release_required_assets.ps1"
    Assert-LastExitCode "Release-required asset manifest"
  }

  if (-not $SkipConfigure) {
    Invoke-GateStep "Configure release build ($ConfigurePreset)" {
      cmake --preset $script:RcConfigurePreset
      Assert-LastExitCode "Configure preset '$script:RcConfigurePreset'"
    }
  }

  if (-not $SkipBuild) {
    Invoke-GateStep "Build release candidate targets ($BuildPreset)" {
      cmake --build --preset $script:RcBuildPreset --target `
        urpg_runtime `
        urpg_editor `
        urpg_audio_smoke `
        urpg_presentation_release_validation `
        urpg_snapshot_canonical_tests `
        urpg_snapshot_renderer_tests `
        urpg_export_unit_tests `
        urpg_spatial_unit_tests `
        urpg_tests `
        urpg_render_tests
      Assert-LastExitCode "Build release candidate targets"
    }
  }

  Invoke-GateStep "Run PR-level tests" {
    $ctestArgs = @("--test-dir", $script:RcBuildPath, "--output-on-failure", "--timeout", "$script:RcCTestTimeoutSeconds", "-L", "^pr$")
    if (-not [string]::IsNullOrWhiteSpace($script:RcConfiguration)) {
      $ctestArgs = @("--test-dir", $script:RcBuildPath, "-C", $script:RcConfiguration, "--output-on-failure", "--timeout", "$script:RcCTestTimeoutSeconds", "-L", "^pr$")
    }
    ctest @ctestArgs
    Assert-LastExitCode "PR-level tests"
  }

  Invoke-GateStep "Run focused presentation validation" {
    if (-not [string]::IsNullOrWhiteSpace($script:RcConfiguration)) {
      & "$PSScriptRoot\run_presentation_gate.ps1" `
        -BuildDirectory $script:RcBuildDirectory `
        -Configuration $script:RcConfiguration `
        -ConfigurePreset $script:RcConfigurePreset `
        -BuildPreset $script:RcBuildPreset `
        -SkipBuild
    } else {
      & "$PSScriptRoot\run_presentation_gate.ps1" `
        -BuildDirectory $script:RcBuildDirectory `
        -ConfigurePreset $script:RcConfigurePreset `
        -BuildPreset $script:RcBuildPreset `
        -SkipBuild
    }
  }

  Invoke-GateStep "Run install smoke" {
    if (-not [string]::IsNullOrWhiteSpace($script:RcConfiguration)) {
      & "$PSScriptRoot\check_install_smoke.ps1" `
        -RepoRoot $script:RcRepoRoot `
        -BuildDirectory $script:RcBuildPath `
        -InstallPrefix $script:RcInstallPath `
        -Configuration $script:RcConfiguration
    } else {
      & "$PSScriptRoot\check_install_smoke.ps1" `
        -RepoRoot $script:RcRepoRoot `
        -BuildDirectory $script:RcBuildPath `
        -InstallPrefix $script:RcInstallPath
    }
  }

  Invoke-GateStep "Run package smoke" {
    if (-not [string]::IsNullOrWhiteSpace($script:RcConfiguration)) {
      & "$PSScriptRoot\check_package_smoke.ps1" `
        -RepoRoot $script:RcRepoRoot `
        -BuildDirectory $script:RcBuildPath `
        -PackageRoot $script:RcPackagePath `
        -Configuration $script:RcConfiguration
    } else {
      & "$PSScriptRoot\check_package_smoke.ps1" `
        -RepoRoot $script:RcRepoRoot `
        -BuildDirectory $script:RcBuildPath `
        -PackageRoot $script:RcPackagePath
    }
  }

  Write-Host "Release candidate gate passed." -ForegroundColor Green
} finally {
  Pop-Location
}
