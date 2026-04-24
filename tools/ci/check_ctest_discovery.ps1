param(
    [string]$BuildDirectory
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\resolve-local-cmake-profile.ps1"

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
    throw "Missing build directory: $buildRoot. Configure and build the test targets before running check_ctest_discovery.ps1."
}

$cachePath = Join-Path $buildRoot "CMakeCache.txt"
if (-not (Test-Path $cachePath)) {
    throw "Missing CMake cache: $cachePath. Configure the build before running check_ctest_discovery.ps1."
}

ctest --test-dir $buildRoot -N
if ($LASTEXITCODE -ne 0) {
    throw "CTest discovery failed for '$buildRoot' with exit code $LASTEXITCODE."
}

Write-Host "CTest discovery completed successfully for '$buildRoot'."
