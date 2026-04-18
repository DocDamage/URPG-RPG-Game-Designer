param(
    [string]$BuildDirectory = "build-local",
    [string]$Configuration = "Debug",
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$buildPath = Join-Path $repoRoot $BuildDirectory

if (-not (Test-Path $buildPath)) {
    throw "Build directory not found: $buildPath"
}

Push-Location $buildPath
try {
    Write-Host "== Validate presentation docs links ==" -ForegroundColor Cyan
    & "$PSScriptRoot\..\docs\check-presentation-doc-links.ps1"
    if ($LASTEXITCODE -ne 0) {
        throw "Presentation docs link validation failed."
    }

    if (-not $SkipBuild) {
        Write-Host "== Build presentation targets ($Configuration) ==" -ForegroundColor Cyan
        cmake --build . --config $Configuration --target urpg_tests urpg_presentation_release_validation
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed for presentation targets."
        }
    }

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
