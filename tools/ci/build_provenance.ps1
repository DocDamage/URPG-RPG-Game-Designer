function Get-UrpgLatestTrackedBuildInput {
  param([Parameter(Mandatory = $true)][string]$RepoRoot)

  $candidates = @(& git -C $RepoRoot ls-files -- CMakeLists.txt CMakePresets.json cmake apps editor engine runtimes)
  if ($LASTEXITCODE -ne 0) {
    throw "Unable to enumerate tracked URPG build inputs."
  }
  $latest = $null
  foreach ($relativePath in $candidates) {
    if ([string]::IsNullOrWhiteSpace($relativePath)) { continue }
    $path = Join-Path $RepoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { continue }
    $item = Get-Item -LiteralPath $path
    if ($null -eq $latest -or $item.LastWriteTimeUtc -gt $latest.LastWriteTimeUtc) {
      $latest = $item
    }
  }
  if ($null -eq $latest) {
    throw "No tracked URPG build inputs were found."
  }
  return $latest
}

function Get-UrpgQualificationBuildProvenance {
  param(
    [Parameter(Mandatory = $true)][string]$Configuration,
    [Parameter(Mandatory = $true)][string]$BuildDirectory,
    [Parameter(Mandatory = $true)][string]$RepoRoot,
    [Parameter(Mandatory = $true)][string]$ExpectedCommit,
    [Parameter(Mandatory = $true)][DateTimeOffset]$GateStarted
  )

  $cache = Join-Path $BuildDirectory "CMakeCache.txt"
  $runtime = Join-Path $BuildDirectory "urpg_runtime.exe"
  if (-not (Test-Path -LiteralPath $cache -PathType Leaf)) {
    throw "The $Configuration build directory is not configured: $BuildDirectory"
  }
  if (-not (Test-Path -LiteralPath $runtime -PathType Leaf)) {
    throw "The $Configuration runtime binary is missing: $runtime"
  }

  $cacheItem = Get-Item -LiteralPath $cache
  $runtimeItem = Get-Item -LiteralPath $runtime
  $latestInput = Get-UrpgLatestTrackedBuildInput -RepoRoot $RepoRoot
  if ($runtimeItem.LastWriteTimeUtc -lt $cacheItem.LastWriteTimeUtc) {
    throw "The $Configuration runtime binary is stale: binary time $($runtimeItem.LastWriteTimeUtc.ToString('o')) is older than configure time $($cacheItem.LastWriteTimeUtc.ToString('o')). Rebuild the candidate."
  }
  if ($runtimeItem.LastWriteTimeUtc -lt $latestInput.LastWriteTimeUtc) {
    throw "The $Configuration runtime binary is stale: binary time $($runtimeItem.LastWriteTimeUtc.ToString('o')) is older than tracked build input '$($latestInput.FullName)' at $($latestInput.LastWriteTimeUtc.ToString('o')). Rebuild the candidate."
  }

  $compilerLine = Select-String -LiteralPath $cache -Pattern '^CMAKE_CXX_COMPILER:FILEPATH=' | Select-Object -First 1
  $dirty = @(& git -C $RepoRoot status --porcelain).Count -ne 0
  return [ordered]@{
    configuration = $Configuration
    preset = if ($Configuration -eq "Debug") { "dev-ninja-debug" } else { "dev-ninja-release" }
    platform = [System.Environment]::OSVersion.Platform.ToString()
    compiler = if ($null -eq $compilerLine) { "unknown" } else { $compilerLine.Line.Split('=', 2)[1] }
    binary_path = $runtimeItem.FullName
    binary_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $runtimeItem.FullName).Hash.ToLowerInvariant()
    source_commit = $ExpectedCommit
    dirty_state = if ($dirty) { "dirty" } else { "clean" }
    configure_utc = $cacheItem.LastWriteTimeUtc.ToString("o")
    binary_utc = $runtimeItem.LastWriteTimeUtc.ToString("o")
    latest_tracked_input = $latestInput.FullName
    latest_tracked_input_utc = $latestInput.LastWriteTimeUtc.ToString("o")
    gate_start_utc = $GateStarted.ToUniversalTime().ToString("o")
  }
}
