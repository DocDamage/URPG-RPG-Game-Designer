param(
    [string]$BuildDirectory
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\resolve-local-cmake-profile.ps1"

function Get-UrpgCMakeCacheValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$CachePath,
        [Parameter(Mandatory = $true)]
        [string]$Key
    )

    if (-not (Test-Path $CachePath)) {
        return $null
    }

    $line = Select-String -Path $CachePath -Pattern "^${Key}(:[^=]+)?=(.*)$" | Select-Object -First 1
    if (-not $line) {
        return $null
    }

    return $line.Matches[0].Groups[2].Value
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
if ([string]::IsNullOrWhiteSpace($BuildDirectory)) {
    $BuildDirectory = (Get-UrpgLocalBuildProfile).BuildDirectory
}

$buildRoot = if ([System.IO.Path]::IsPathRooted($BuildDirectory)) {
    $BuildDirectory
} else {
    Join-Path $repoRoot $BuildDirectory
}

if (-not (Test-Path $buildRoot)) {
    throw "Missing build directory: $buildRoot. Build an OpenGL-enabled profile before running check_renderer_backed_visual_capture.ps1."
}

$cachePath = Join-Path $buildRoot "CMakeCache.txt"
$skipOpenGL = Get-UrpgCMakeCacheValue -CachePath $cachePath -Key "URPG_SKIP_OPENGL"
if ($skipOpenGL -eq "ON") {
    throw "Build directory '$buildRoot' is configured with URPG_SKIP_OPENGL=ON. Renderer-backed visual capture requires a non-headless OpenGL-enabled build."
}

$matches = Get-ChildItem -Path $buildRoot -Recurse -File | Where-Object {
    $_.BaseName -eq "urpg_snapshot_renderer_tests" -and ($_.Extension -eq ".exe" -or [string]::IsNullOrEmpty($_.Extension))
} | Sort-Object -Property @(
    @{ Expression = "LastWriteTime"; Descending = $true },
    @{ Expression = "FullName"; Descending = $false }
)

if (-not $matches) {
    throw "Could not find urpg_snapshot_renderer_tests under $buildRoot. Build an OpenGL-enabled profile before running check_renderer_backed_visual_capture.ps1."
}

$testExecutable = ($matches | Select-Object -First 1).FullName
$buildDir = Split-Path $testExecutable -Parent
$sdlRuntimeDir = Join-Path $buildDir "_deps\sdl2-build"

if (Test-Path $sdlRuntimeDir) {
    $env:PATH = "$sdlRuntimeDir;$env:PATH"
}

$previousGoldenThreshold = $env:URPG_RENDERER_BACKED_GOLDEN_THRESHOLD
if ($env:CI -eq "true" -and [string]::IsNullOrWhiteSpace($previousGoldenThreshold)) {
    $env:URPG_RENDERER_BACKED_GOLDEN_THRESHOLD = "100"
}

& $testExecutable "[snapshot][renderer][visual_capture]" --reporter compact
$env:URPG_RENDERER_BACKED_GOLDEN_THRESHOLD = $previousGoldenThreshold
if ($LASTEXITCODE -ne 0) {
    throw "Renderer-backed visual capture tests failed with exit code $LASTEXITCODE."
}

Write-Host "Renderer-backed visual capture gate passed."
