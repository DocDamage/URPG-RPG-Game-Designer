param(
  [string]$RepoRoot = (Get-Location).Path,
  [string]$BuildDirectory = "build/dev-ninja-release",
  [string]$InstallPrefix = "",
  [string]$Configuration = "",
  [switch]$NoClean
)

$ErrorActionPreference = "Stop"

function Resolve-UnderRoot {
  param(
    [string]$Root,
    [string]$Path
  )

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }

  return [System.IO.Path]::GetFullPath((Join-Path $Root $Path))
}

$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$BuildDirectory = Resolve-UnderRoot -Root $RepoRoot -Path $BuildDirectory
if ([string]::IsNullOrWhiteSpace($InstallPrefix)) {
  $InstallPrefix = Join-Path $BuildDirectory "install-smoke"
} else {
  $InstallPrefix = Resolve-UnderRoot -Root $RepoRoot -Path $InstallPrefix
}

$cachePath = Join-Path $BuildDirectory "CMakeCache.txt"
if (-not (Test-Path -LiteralPath $cachePath -PathType Leaf)) {
  throw "Build directory is not configured: $BuildDirectory"
}

if ((Test-Path -LiteralPath $InstallPrefix) -and -not $NoClean) {
  Remove-Item -LiteralPath $InstallPrefix -Recurse -Force
}
New-Item -ItemType Directory -Path $InstallPrefix -Force | Out-Null

$components = @("Runtime", "RuntimeData", "Docs")
foreach ($component in $components) {
  $installArgs = @("--install", $BuildDirectory, "--prefix", $InstallPrefix, "--component", $component)
  if (-not [string]::IsNullOrWhiteSpace($Configuration)) {
    $installArgs += @("--config", $Configuration)
  }

  & cmake @installArgs
  if ($LASTEXITCODE -ne 0) {
    throw "cmake --install --component $component failed with exit code $LASTEXITCODE"
  }
}

$exeSuffix = ""
if ($IsWindows -or $env:OS -eq "Windows_NT") {
  $exeSuffix = ".exe"
}

$requiredPaths = @(
  "bin/urpg_runtime$exeSuffix",
  "bin/urpg_editor$exeSuffix",
  "bin/urpg_audio_smoke$exeSuffix",
  "share/icons/hicolor/256x256/apps/urpg_runtime.png",
  "share/icons/hicolor/256x256/apps/urpg_editor.png",
  "share/applications/urpg-runtime.desktop",
  "share/applications/urpg-editor.desktop",
  "share/urpg/content/schemas/project.schema.json",
  "share/urpg/content/templates/jrpg_starter.json",
  "share/urpg/content/readiness/readiness_status.json",
  "share/urpg/content/level_libraries/starter_dungeon.json",
  "share/urpg/imports/manifests/asset_bundles/asset_bundle.schema.json",
  "share/doc/urpg/README.md",
  "share/doc/urpg/LICENSE",
  "share/doc/urpg/CHANGELOG.md",
  "share/doc/urpg/PRIVACY_POLICY.md",
  "share/doc/urpg/THIRD_PARTY_NOTICES.md",
  "share/doc/urpg/EULA.md",
  "share/doc/urpg/CREDITS.md",
  "share/doc/urpg/release/RELEASE_PACKAGING.md"
)

$missing = @()
foreach ($relativePath in $requiredPaths) {
  $candidate = Join-Path $InstallPrefix $relativePath
  if (-not (Test-Path -LiteralPath $candidate)) {
    $missing += $relativePath
  }
}

if ($exeSuffix -eq ".exe") {
  $sdlRuntimeCandidates = @("bin/SDL2.dll", "bin/SDL2d.dll")
  $hasSdlRuntime = $false
  foreach ($candidate in $sdlRuntimeCandidates) {
    if (Test-Path -LiteralPath (Join-Path $InstallPrefix $candidate) -PathType Leaf) {
      $hasSdlRuntime = $true
      break
    }
  }
  if (-not $hasSdlRuntime) {
    $missing += "bin/SDL2.dll or bin/SDL2d.dll"
  }
}

if ($missing.Count -gt 0) {
  throw "Install layout is missing required path(s): $($missing -join ', ')"
}

$runtimeExe = Join-Path $InstallPrefix "bin/urpg_runtime$exeSuffix"
$installedProjectRoot = Join-Path $InstallPrefix "share/urpg"
Push-Location $InstallPrefix
try {
  & $runtimeExe --headless --frames 1 --project-root $installedProjectRoot
  if ($LASTEXITCODE -ne 0) {
    throw "Installed runtime smoke failed with exit code $LASTEXITCODE"
  }
} finally {
  Pop-Location
}

Write-Host "Install smoke passed: $InstallPrefix"
