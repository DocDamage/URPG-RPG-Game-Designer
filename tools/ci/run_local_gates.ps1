param(
    [string]$ConfigurePreset = "ci",
    [string]$BuildPreset = "ci-release",
    [string]$PresentationConfiguration = "Release",
    [switch]$SkipBuild,
    [switch]$SkipPresentationGate
)

$ErrorActionPreference = "Stop"

Write-Host "== Validate waivers ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_waivers.ps1"

Write-Host "== Validate Wave 1 subsystem checklist sync ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_wave1_spec_checklists.ps1"

Write-Host "== Validate presentation docs links ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check-presentation-doc-links.ps1"

Write-Host "== Configure: $ConfigurePreset ==" -ForegroundColor Cyan
cmake --preset $ConfigurePreset

if (-not $SkipBuild) {
    Write-Host "== Build: $BuildPreset ==" -ForegroundColor Cyan
    cmake --build --preset $BuildPreset
}

$testDir = "build/$ConfigurePreset"

if (-not $SkipPresentationGate) {
    Write-Host "== Focused presentation gate ==" -ForegroundColor Cyan
    & "$PSScriptRoot\run_presentation_gate.ps1" `
        -BuildDirectory $testDir `
        -Configuration $PresentationConfiguration `
        -SkipBuild
}

Write-Host "== Validate curated RPG Maker plugin drop-ins ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\rpgmaker\validate-plugin-dropins.ps1" `
    -PluginRoot "third_party\rpgmaker-mz\steam-dlc\plugin-dropins-curated\js\plugins" `
    -ReportPrefix "plugin_dropins_curated_ci_validation" `
    -FailOnError

Write-Host "== Gate 1 (pr) ==" -ForegroundColor Cyan
ctest --test-dir $testDir --output-on-failure -L pr

Write-Host "== Gate 2 (nightly) ==" -ForegroundColor Cyan
ctest --test-dir $testDir --output-on-failure -L nightly

Write-Host "== Gate 3 (weekly) ==" -ForegroundColor Cyan
ctest --test-dir $testDir --output-on-failure -L weekly

Write-Host "All gates passed." -ForegroundColor Green
