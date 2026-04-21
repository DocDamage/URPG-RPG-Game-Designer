param(
    [string]$BuildDirectory,
    [string]$Configuration,
    [string]$ConfigurePreset,
    [string]$BuildPreset,
    [switch]$SkipBuild
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

$usingLocalProfileDefaults = [string]::IsNullOrWhiteSpace($BuildDirectory) -or
    [string]::IsNullOrWhiteSpace($Configuration) -or
    [string]::IsNullOrWhiteSpace($ConfigurePreset) -or
[string]::IsNullOrWhiteSpace($BuildPreset)

if ($usingLocalProfileDefaults) {
    $localProfile = Get-UrpgLocalBuildProfile
    if ([string]::IsNullOrWhiteSpace($BuildDirectory)) {
        $BuildDirectory = $localProfile.BuildDirectory
    }
    if ([string]::IsNullOrWhiteSpace($Configuration)) {
        $Configuration = $localProfile.Configuration
    }
    if ([string]::IsNullOrWhiteSpace($ConfigurePreset)) {
        $ConfigurePreset = $localProfile.ConfigurePreset
    }
    if ([string]::IsNullOrWhiteSpace($BuildPreset)) {
        $BuildPreset = $localProfile.BuildPreset
    }
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$buildPath = Join-Path $repoRoot $BuildDirectory
$cachePath = Join-Path $buildPath "CMakeCache.txt"
$shouldConfigure = -not (Test-Path $buildPath)

if (-not $shouldConfigure -and $usingLocalProfileDefaults) {
    $cachedGenerator = Get-UrpgCMakeCacheValue -CachePath $cachePath -Key "CMAKE_GENERATOR"
    if ([string]::IsNullOrWhiteSpace($cachedGenerator) -or $cachedGenerator -ne $localProfile.Generator) {
        Write-Host "== Reset stale build tree: $BuildDirectory ==" -ForegroundColor Yellow
        Remove-Item -Recurse -Force -LiteralPath $buildPath
        $shouldConfigure = $true
    }
}

if ($shouldConfigure) {
    Write-Host "== Configure: $ConfigurePreset ==" -ForegroundColor Cyan
    cmake --preset $ConfigurePreset
    if ($LASTEXITCODE -ne 0) {
        throw "Configure failed for preset '$ConfigurePreset'."
    }
}

$ErrorActionPreference = "Stop"

Write-Host "== Validate presentation docs links ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check-presentation-doc-links.ps1"
if ($LASTEXITCODE -ne 0) {
    throw "Presentation docs link validation failed."
}

if (-not $SkipBuild) {
    Write-Host "== Build presentation targets ($Configuration) ==" -ForegroundColor Cyan
    cmake --build --preset $BuildPreset --target urpg_tests urpg_presentation_release_validation
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed for presentation targets."
    }
}

Push-Location $buildPath
try {
    Write-Host "== Presentation gate ==" -ForegroundColor Cyan
    ctest -C $Configuration -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure
    if ($LASTEXITCODE -ne 0) {
        throw "Presentation gate failed."
    }

    Write-Host "Presentation gate passed." -ForegroundColor Green
}
finally {
    Pop-Location
}
