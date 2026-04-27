param(
  [string]$RepoRoot = (Get-Location).Path,
  [string]$BuildDirectory = "build/dev-ninja-release",
  [string]$PackageRoot = "build/package-smoke",
  [string]$Generator = "ZIP",
  [string]$Configuration = ""
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

function Assert-ZipContains {
  param(
    [Parameter(Mandatory = $true)]
    [string]$ArchivePath,
    [Parameter(Mandatory = $true)]
    [string[]]$RequiredEntries
  )

  Add-Type -AssemblyName System.IO.Compression.FileSystem
  $archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
  try {
    $entries = @{}
    foreach ($entry in $archive.Entries) {
      $normalized = $entry.FullName.Replace("\", "/")
      $entries[$normalized] = $true
    }

    $missing = @()
    foreach ($required in $RequiredEntries) {
      $found = $false
      foreach ($entryName in $entries.Keys) {
        if ($entryName.EndsWith($required)) {
          $found = $true
          break
        }
      }
      if (-not $found) {
        $missing += $required
      }
    }

    if ($missing.Count -gt 0) {
      throw "Archive '$ArchivePath' is missing required entry suffix(es): $($missing -join ', ')"
    }
  } finally {
    $archive.Dispose()
  }
}

function Assert-ZipExcludesDevBootstrapMarkers {
  param(
    [Parameter(Mandatory = $true)]
    [string]$ArchivePath
  )

  Add-Type -AssemblyName System.IO.Compression.FileSystem
  $archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
  try {
    foreach ($entry in $archive.Entries) {
      $normalized = $entry.FullName.Replace("\", "/")
      if ($normalized -match "(^|/)DevBootstrap(/|$)" -or $normalized -match "dev_bootstrap") {
        throw "Release package archive '$ArchivePath' contains DevBootstrap marker entry '$normalized'."
      }

      if ($entry.Length -gt 0 -and $entry.Length -lt 1048576) {
        $stream = $entry.Open()
        try {
          $reader = New-Object System.IO.StreamReader($stream)
          try {
            $text = $reader.ReadToEnd()
            if ($text.Contains("URPG DEV BOOTSTRAP ONLY") -or $text.Contains('"mode": "DevBootstrap"')) {
              throw "Release package archive '$ArchivePath' contains DevBootstrap marker content in '$normalized'."
            }
          } finally {
            $reader.Dispose()
          }
        } finally {
          $stream.Dispose()
        }
      }
    }
  } finally {
    $archive.Dispose()
  }
}

$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$BuildDirectory = Resolve-UnderRoot -Root $RepoRoot -Path $BuildDirectory
$PackageRoot = Resolve-UnderRoot -Root $RepoRoot -Path $PackageRoot

$cpackConfig = Join-Path $BuildDirectory "CPackConfig.cmake"
if (-not (Test-Path -LiteralPath $cpackConfig -PathType Leaf)) {
  throw "CPack config does not exist. Configure the build tree first: $cpackConfig"
}

if (Test-Path -LiteralPath $PackageRoot) {
  Remove-Item -LiteralPath $PackageRoot -Recurse -Force
}
New-Item -ItemType Directory -Path $PackageRoot -Force | Out-Null

$cpackArgs = @("--config", $cpackConfig, "-G", $Generator, "-B", $PackageRoot)
if (-not [string]::IsNullOrWhiteSpace($Configuration)) {
  $cpackArgs += @("-C", $Configuration)
}

& cpack @cpackArgs
if ($LASTEXITCODE -ne 0) {
  throw "cpack failed with exit code $LASTEXITCODE"
}

if ($Generator -ne "ZIP") {
  Write-Host "Package smoke generated '$Generator' artifacts but archive-content validation currently runs only for ZIP."
  return
}

$archives = Get-ChildItem -LiteralPath $PackageRoot -Filter "*.zip" -File
if ($archives.Count -lt 3) {
  throw "Expected Runtime, RuntimeData, and Docs ZIP archives in $PackageRoot; found $($archives.Count)."
}

$runtimeArchive = $archives | Where-Object { $_.Name -match "Runtime" } | Select-Object -First 1
$runtimeDataArchive = $archives | Where-Object { $_.Name -match "RuntimeData" } | Select-Object -First 1
$docsArchive = $archives | Where-Object { $_.Name -match "Docs" } | Select-Object -First 1

if ($null -eq $runtimeArchive -or $null -eq $runtimeDataArchive -or $null -eq $docsArchive) {
  throw "Could not identify Runtime, RuntimeData, and Docs component archives in $PackageRoot."
}

Assert-ZipExcludesDevBootstrapMarkers -ArchivePath $runtimeArchive.FullName
Assert-ZipExcludesDevBootstrapMarkers -ArchivePath $runtimeDataArchive.FullName
Assert-ZipExcludesDevBootstrapMarkers -ArchivePath $docsArchive.FullName

$exeSuffix = ""
if ($IsWindows -or $env:OS -eq "Windows_NT") {
  $exeSuffix = ".exe"
}

Assert-ZipContains -ArchivePath $runtimeArchive.FullName -RequiredEntries @(
  "bin/urpg_runtime$exeSuffix",
  "bin/urpg_editor$exeSuffix",
  "bin/urpg_audio_smoke$exeSuffix",
  "share/icons/hicolor/256x256/apps/urpg_runtime.png",
  "share/icons/hicolor/256x256/apps/urpg_editor.png",
  "share/applications/urpg-runtime.desktop",
  "share/applications/urpg-editor.desktop"
)

Assert-ZipContains -ArchivePath $runtimeDataArchive.FullName -RequiredEntries @(
  "share/urpg/content/schemas/project.schema.json",
  "share/urpg/content/templates/jrpg_starter.json",
  "share/urpg/imports/manifests/asset_bundles/asset_bundle.schema.json"
)

Assert-ZipContains -ArchivePath $docsArchive.FullName -RequiredEntries @(
  "share/doc/urpg/README.md",
  "share/doc/urpg/LICENSE",
  "share/doc/urpg/THIRD_PARTY_NOTICES.md",
  "share/doc/urpg/EULA.md",
  "share/doc/urpg/CREDITS.md",
  "share/doc/urpg/PRIVACY_POLICY.md"
)

Write-Host "Package smoke passed: $PackageRoot"
