param([string]$BuildDirectory = "build/dev-ninja-debug")

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$buildRoot = if ([System.IO.Path]::IsPathRooted($BuildDirectory)) {
  [System.IO.Path]::GetFullPath($BuildDirectory)
} else {
  [System.IO.Path]::GetFullPath((Join-Path $repoRoot $BuildDirectory))
}
. (Join-Path $PSScriptRoot "build_provenance.ps1")

$runtime = Join-Path $buildRoot "urpg_runtime.exe"
if (-not (Test-Path -LiteralPath $runtime -PathType Leaf)) {
  throw "Build provenance self-test requires a runtime binary: $runtime"
}
$originalTime = (Get-Item -LiteralPath $runtime).LastWriteTimeUtc
try {
  $latestInput = Get-UrpgLatestTrackedBuildInput -RepoRoot $repoRoot
  (Get-Item -LiteralPath $runtime).LastWriteTimeUtc = $latestInput.LastWriteTimeUtc.AddSeconds(-1)
  $rejected = $false
  try {
    [void](Get-UrpgQualificationBuildProvenance -Configuration "Debug" -BuildDirectory $buildRoot `
      -RepoRoot $repoRoot -ExpectedCommit ((& git -C $repoRoot rev-parse HEAD).Trim()) `
      -GateStarted ([DateTimeOffset]::UtcNow))
  } catch {
    if ($_.Exception.Message -notmatch "binary is stale") { throw }
    Write-Host $_.Exception.Message
    $rejected = $true
  }
  if (-not $rejected) { throw "Deliberately stale runtime binary was accepted." }
} finally {
  (Get-Item -LiteralPath $runtime).LastWriteTimeUtc = $originalTime
}
Write-Host "Build provenance self-test passed: deliberately stale runtime binary was rejected and timestamp restored."
