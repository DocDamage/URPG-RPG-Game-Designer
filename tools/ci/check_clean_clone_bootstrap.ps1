param(
  [string]$ConfigurePreset = "dev-ninja-debug",
  [string]$BuildPreset = "dev-debug",
  [string]$SourceBuildDirectory = "build/dev-ninja-debug",
  [string]$ReportPath = "build/reports/product_completion/clean_clone_bootstrap.json",
  [switch]$KeepClone
)

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
if (@(& git -C $repoRoot status --porcelain).Count -ne 0) {
  throw "Clean-clone bootstrap requires a clean worktree so the disposable clone represents the candidate exactly."
}
$head = (& git -C $repoRoot rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0) { throw "Unable to resolve candidate commit." }
$sourceBuild = if ([System.IO.Path]::IsPathRooted($SourceBuildDirectory)) {
  [System.IO.Path]::GetFullPath($SourceBuildDirectory)
} else { [System.IO.Path]::GetFullPath((Join-Path $repoRoot $SourceBuildDirectory)) }
if (-not (Test-Path -LiteralPath (Join-Path $sourceBuild "CTestTestfile.cmake") -PathType Leaf)) {
  throw "Source candidate test discovery is unavailable: $sourceBuild"
}

function Get-DiscoveredTests([string]$BuildDirectory) {
  $jsonText = (& ctest --test-dir $BuildDirectory -N --show-only=json-v1) -join [Environment]::NewLine
  if ($LASTEXITCODE -ne 0) { throw "CTest discovery failed in $BuildDirectory" }
  $json = $jsonText | ConvertFrom-Json
  return @($json.tests | ForEach-Object { $_.name } | Sort-Object -Unique)
}

$sourceTests = Get-DiscoveredTests -BuildDirectory $sourceBuild
$tempRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath())
$cloneRoot = Join-Path $tempRoot ("urpg-clean-clone-{0:yyyyMMdd-HHmmss}-{1}" -f (Get-Date), $PID)
$cloneBuild = Join-Path $cloneRoot "build\$ConfigurePreset"
$started = [DateTimeOffset]::UtcNow
$cloneTests = @()
try {
  & git clone --no-local --no-hardlinks $repoRoot $cloneRoot
  if ($LASTEXITCODE -ne 0) { throw "Disposable local clone failed." }
  & git -C $cloneRoot checkout --detach $head
  if ($LASTEXITCODE -ne 0) { throw "Disposable clone could not check out $head." }
  & (Join-Path $cloneRoot "tools\ci\check_release_required_assets.ps1")
  if ($LASTEXITCODE -ne 0) { throw "Disposable clone release-required asset hydration/validation failed." }
  Push-Location $cloneRoot
  try {
    & cmake --fresh --preset $ConfigurePreset
    if ($LASTEXITCODE -ne 0) { throw "Disposable clone configure failed." }
    & cmake --build --preset $BuildPreset
    if ($LASTEXITCODE -ne 0) { throw "Disposable clone build failed." }
  } finally { Pop-Location }
  $cloneTests = Get-DiscoveredTests -BuildDirectory $cloneBuild
  $missing = @($sourceTests | Where-Object { $_ -notin $cloneTests })
  $unexpected = @($cloneTests | Where-Object { $_ -notin $sourceTests })
  if ($missing.Count -gt 0 -or $unexpected.Count -gt 0) {
    throw "Disposable clone discovered a different test set: missing=$($missing.Count), unexpected=$($unexpected.Count)."
  }

  $resolvedReport = if ([System.IO.Path]::IsPathRooted($ReportPath)) { $ReportPath } else { Join-Path $repoRoot $ReportPath }
  New-Item -ItemType Directory -Force -Path (Split-Path -Parent $resolvedReport) | Out-Null
  $report = [ordered]@{
    schema = "urpg.clean_clone_bootstrap.v1"
    status = "passed"
    sourceCommit = $head
    cleanWorktree = $true
    configurePreset = $ConfigurePreset
    buildPreset = $BuildPreset
    startedUtc = $started.ToString("o")
    completedUtc = [DateTimeOffset]::UtcNow.ToString("o")
    sourceDiscoveredTestCount = $sourceTests.Count
    cloneDiscoveredTestCount = $cloneTests.Count
    discoveredTestsSha256 = [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData(
      [Text.Encoding]::UTF8.GetBytes(($cloneTests -join "`n")))).ToLowerInvariant()
  }
  [System.IO.File]::WriteAllText($resolvedReport, ($report | ConvertTo-Json -Depth 6) + [Environment]::NewLine)
  Write-Host "Clean-clone bootstrap passed with $($cloneTests.Count) discovered tests: $resolvedReport"
} finally {
  if (-not $KeepClone -and (Test-Path -LiteralPath $cloneRoot)) {
    $resolvedClone = [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $cloneRoot).Path)
    if (-not $resolvedClone.StartsWith($tempRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
      throw "Refusing to remove clone outside the system temp root: $resolvedClone"
    }
    Remove-Item -LiteralPath $resolvedClone -Recurse -Force
  } elseif ($KeepClone) {
    Write-Host "Disposable clone retained: $cloneRoot"
  }
}
