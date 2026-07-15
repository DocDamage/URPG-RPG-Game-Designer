param(
  [string]$DebugBuildDirectory = "build/dev-ninja-debug",
  [string]$ReleaseBuildDirectory = "build/dev-ninja-release",
  [string]$PackageRoot = "build/package-smoke",
  [string]$ExpectedCommit = "",
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

function Resolve-RepoPath {
  param([Parameter(Mandatory = $true)][string]$Path)
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
}

function Invoke-QualificationCommand {
  param(
    [Parameter(Mandatory = $true)][string]$Id,
    [Parameter(Mandatory = $true)][string]$Description,
    [Parameter(Mandatory = $true)][scriptblock]$Command
  )

  $started = [DateTimeOffset]::UtcNow
  try {
    & $Command
    if ($null -ne $LASTEXITCODE -and $LASTEXITCODE -ne 0) {
      throw "Native command exited with code $LASTEXITCODE."
    }
  } catch {
    throw "PFU-I1 qualification '$Id' ($Description) failed: $($_.Exception.Message)"
  }
  $script:commands += [ordered]@{
    id = $Id
    description = $Description
    status = "passed"
    duration_ms = [int]([DateTimeOffset]::UtcNow - $started).TotalMilliseconds
  }
}

function Get-BinaryProvenance {
  param(
    [Parameter(Mandatory = $true)][string]$Configuration,
    [Parameter(Mandatory = $true)][string]$BuildDirectory
  )

  $cache = Join-Path $BuildDirectory "CMakeCache.txt"
  if (-not (Test-Path -LiteralPath $cache -PathType Leaf)) {
    throw "The $Configuration build directory is not configured: $BuildDirectory"
  }
  $runtime = Join-Path $BuildDirectory "urpg_runtime.exe"
  if (-not (Test-Path -LiteralPath $runtime -PathType Leaf)) {
    throw "The $Configuration runtime binary is missing: $runtime"
  }
  $compilerLine = Select-String -LiteralPath $cache -Pattern '^CMAKE_CXX_COMPILER:FILEPATH=' | Select-Object -First 1
  return [ordered]@{
    configuration = $Configuration
    preset = if ($Configuration -eq "Debug") { "dev-ninja-debug" } else { "dev-ninja-release" }
    platform = [System.Environment]::OSVersion.Platform.ToString()
    compiler = if ($null -eq $compilerLine) { "unknown" } else { $compilerLine.Line.Split('=', 2)[1] }
    binary_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $runtime).Hash.ToLowerInvariant()
    source_commit = $ExpectedCommit
  }
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$debugBuild = Resolve-RepoPath $DebugBuildDirectory
$releaseBuild = Resolve-RepoPath $ReleaseBuildDirectory
$packageRoot = Resolve-RepoPath $PackageRoot
if ([string]::IsNullOrWhiteSpace($ExpectedCommit)) {
  $ExpectedCommit = (& git -C $repoRoot rev-parse HEAD).Trim()
}
if ([string]::IsNullOrWhiteSpace($ExpectedCommit)) {
  throw "Unable to resolve the expected source commit."
}
if ((& git -C $repoRoot rev-parse HEAD).Trim() -ne $ExpectedCommit) {
  throw "ExpectedCommit does not match HEAD. Qualify the checked-out commit, not a different revision."
}
if (@(& git -C $repoRoot status --porcelain).Count -ne 0) {
  throw "PFU-I1 qualification requires a clean worktree. Commit or remove all local changes first."
}

$targetReportPath = Join-Path $debugBuild "creator_journey_qualification_report.json"
$targetProvenancePath = Join-Path $debugBuild "creator_journey_qualification_provenance.json"
$packageEvidencePath = Join-Path $debugBuild "creator_journey_qualification_package_smoke.json"
Remove-Item -LiteralPath $targetReportPath, $targetProvenancePath, $packageEvidencePath -Force -ErrorAction SilentlyContinue

$commands = @()
if (-not $SkipBuild) {
  Invoke-QualificationCommand "build_debug" "Build the clean Debug target" {
    & cmake --build $debugBuild
  }
  Invoke-QualificationCommand "build_release" "Build the clean Release target" {
    & cmake --build $releaseBuild
  }
}

$verticalSliceGate = Join-Path $repoRoot "tools\ci\check_creator_vertical_slice.ps1"
if (-not (Test-Path -LiteralPath $verticalSliceGate -PathType Leaf)) {
  throw "PFU-I1 qualification requires the native M9 vertical-slice gate: $verticalSliceGate"
}

Invoke-QualificationCommand "creator_journey" "Run the historical creator-journey baseline" {
  & ctest --test-dir $debugBuild -R "creator journey" --output-on-failure
  & (Join-Path $repoRoot "tools\ci\check_creator_journey.ps1") -BuildDirectory $debugBuild
}
Invoke-QualificationCommand "creator_vertical_slice" "Run the creator-authored M9 vertical slice" {
  & ctest --test-dir $debugBuild -R "creator vertical slice" --output-on-failure
  & $verticalSliceGate -BuildDirectory $debugBuild
}
Invoke-QualificationCommand "spatial" "Run the spatial authoring suite" {
  & ctest --test-dir $debugBuild -L spatial --output-on-failure
}
Invoke-QualificationCommand "governed_assets" "Verify governed asset intake" {
  & (Join-Path $repoRoot "tools\ci\check_asset_library_governance.ps1") -RepoRoot $repoRoot
}
Invoke-QualificationCommand "persistence_recovery" "Run persistence and recovery coverage" {
  & ctest --test-dir $debugBuild -R "(persistence|recovery)" --output-on-failure
}
Invoke-QualificationCommand "accessibility_input" "Verify accessibility and input governance" {
  & (Join-Path $repoRoot "tools\ci\check_accessibility_governance.ps1")
  & (Join-Path $repoRoot "tools\ci\check_input_governance.ps1")
}
Invoke-QualificationCommand "package_install" "Run package and install smoke" {
  & (Join-Path $repoRoot "tools\ci\check_package_smoke.ps1") -RepoRoot $repoRoot -BuildDirectory $releaseBuild -PackageRoot $packageRoot
  & (Join-Path $repoRoot "tools\ci\check_install_smoke.ps1") -RepoRoot $repoRoot -BuildDirectory $releaseBuild
}
$buildProvenance = @(
  (Get-BinaryProvenance -Configuration "Debug" -BuildDirectory $debugBuild),
  (Get-BinaryProvenance -Configuration "Release" -BuildDirectory $releaseBuild)
)
$packageEvidence = [ordered]@{
  schema = "urpg.creator_journey_qualification_package_smoke.v1"
  status = "passed"
  source_commit = $ExpectedCommit
  package_root = $packageRoot
  generated_utc = [DateTimeOffset]::UtcNow.ToString("o")
}
[System.IO.File]::WriteAllText($packageEvidencePath, ($packageEvidence | ConvertTo-Json -Depth 6) + [Environment]::NewLine)
$targetProvenance = [ordered]@{
  schema = "urpg.creator_journey_qualification_provenance.v1"
  source_commit = $ExpectedCommit
  clean_worktree = $true
  builds = $buildProvenance
}
[System.IO.File]::WriteAllText($targetProvenancePath, ($targetProvenance | ConvertTo-Json -Depth 8) + [Environment]::NewLine)
Invoke-QualificationCommand "target_creator_journey" "Emit native target creator-journey evidence" {
  & ctest --test-dir $debugBuild -R "creator journey qualification" --output-on-failure
}
Invoke-QualificationCommand "strict_creator_qualification" "Validate the complete target creator-journey report" {
  & (Join-Path $repoRoot "tools\ci\check_creator_journey_qualification.ps1") -BuildDirectory $debugBuild -ExpectedCommit $ExpectedCommit
}

$manifest = [ordered]@{
  schema = "urpg.pfu_i1_qualification_manifest.v1"
  qualification_id = "pfu_i1_native_creator_baseline"
  status = "passed"
  source_commit = $ExpectedCommit
  clean_worktree = $true
  generated_utc = [DateTimeOffset]::UtcNow.ToString("o")
  builds = $buildProvenance
  commands = $commands
  target_report = [ordered]@{
    path = $targetReportPath
    sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $targetReportPath).Hash.ToLowerInvariant()
  }
}
$manifestPath = Join-Path $debugBuild "pfu_i1_qualification_manifest.json"
[System.IO.File]::WriteAllText($manifestPath, ($manifest | ConvertTo-Json -Depth 8) + [Environment]::NewLine)
Write-Host "PFU-I1 qualification passed: $manifestPath"
