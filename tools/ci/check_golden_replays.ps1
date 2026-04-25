param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path,
  [string]$ExpectedPath = "",
  [string]$CurrentPath = "",
  [string]$ProjectVersion = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ExpectedPath)) {
  $ExpectedPath = Join-Path $RepoRoot "content\fixtures\golden_replays\expected.json"
}
if ([string]::IsNullOrWhiteSpace($CurrentPath)) {
  $CurrentPath = Join-Path $RepoRoot "content\fixtures\golden_replays\current.json"
}

if (-not (Test-Path $ExpectedPath)) {
  throw "Missing expected golden replay file: $ExpectedPath"
}
if (-not (Test-Path $CurrentPath)) {
  throw "Missing current golden replay file: $CurrentPath"
}

$expectedRoot = Get-Content -Raw -Path $ExpectedPath | ConvertFrom-Json
$currentRoot = Get-Content -Raw -Path $CurrentPath | ConvertFrom-Json

if ([string]::IsNullOrWhiteSpace($ProjectVersion)) {
  $ProjectVersion = [string]$expectedRoot.project_version
}
if ([string]::IsNullOrWhiteSpace($ProjectVersion)) {
  throw "Golden replay validation requires a project_version."
}

function ConvertTo-ReplayMap($items, [string]$sourceName) {
  $map = @{}
  foreach ($item in @($items)) {
    if ([string]::IsNullOrWhiteSpace([string]$item.id)) {
      throw "$sourceName contains a replay without an id."
    }
    $map[[string]$item.id] = $item
  }
  return $map
}

function Get-StateHashMap($replay) {
  $map = @{}
  if ($null -eq $replay.state_hashes) {
    return $map
  }
  foreach ($property in $replay.state_hashes.PSObject.Properties) {
    $map[[int64]$property.Name] = [string]$property.Value
  }
  return $map
}

$expected = ConvertTo-ReplayMap $expectedRoot.replays "expected golden replay file"
$current = ConvertTo-ReplayMap $currentRoot.replays "current golden replay file"
$ids = @($expected.Keys + $current.Keys | Sort-Object -Unique)
$failures = @()

foreach ($id in $ids) {
  if (-not $expected.ContainsKey($id)) {
    $failures += "${id}: missing expected golden baseline"
    continue
  }
  if (-not $current.ContainsKey($id)) {
    $failures += "${id}: missing current replay artifact"
    continue
  }

  $expectedReplay = $expected[$id]
  $currentReplay = $current[$id]
  if ([string]$expectedReplay.project_version -ne $ProjectVersion -or [string]$currentReplay.project_version -ne $ProjectVersion) {
    $failures += "${id}: stale project version expected=$($expectedReplay.project_version) current=$($currentReplay.project_version) required=$ProjectVersion"
    continue
  }

  $expectedHashes = Get-StateHashMap $expectedReplay
  $currentHashes = Get-StateHashMap $currentReplay
  $ticks = @($expectedHashes.Keys + $currentHashes.Keys | Sort-Object -Unique)
  foreach ($tick in $ticks) {
    $expectedHash = $expectedHashes[$tick]
    $currentHash = $currentHashes[$tick]
    if ($expectedHash -ne $currentHash) {
      $failures += "${id}: state hash diverged at tick $tick expected=$expectedHash current=$currentHash"
      break
    }
  }
}

if ($failures.Count -gt 0) {
  throw "Golden replay validation failed: $($failures -join '; ')"
}

Write-Host "Golden replay validation passed: $($ids.Count) replay artifact(s) matched."
