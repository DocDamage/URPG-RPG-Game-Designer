param(
    [string]$ConfigurePreset,
    [string]$BuildPreset,
    [string]$PresentationConfiguration,
    [switch]$SkipBuild,
    [switch]$SkipPresentationGate
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\resolve-local-cmake-profile.ps1"

if ([string]::IsNullOrWhiteSpace($ConfigurePreset) -or
    [string]::IsNullOrWhiteSpace($BuildPreset) -or
    [string]::IsNullOrWhiteSpace($PresentationConfiguration)) {
    $localProfile = Get-UrpgLocalBuildProfile
    if ([string]::IsNullOrWhiteSpace($ConfigurePreset)) {
        $ConfigurePreset = $localProfile.ConfigurePreset
    }
    if ([string]::IsNullOrWhiteSpace($BuildPreset)) {
        $BuildPreset = $localProfile.BuildPreset
    }
    if ([string]::IsNullOrWhiteSpace($PresentationConfiguration)) {
        $PresentationConfiguration = $localProfile.Configuration
    }
}

Write-Host "== Validate waivers ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_waivers.ps1"

Write-Host "== Validate Wave 1 subsystem checklist sync ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_wave1_spec_checklists.ps1"

Write-Host "== Validate presentation docs links ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check-presentation-doc-links.ps1"

Write-Host "== Validate release readiness records ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_release_readiness.ps1"

Write-Host "== Validate schema changelog governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_schema_changelog.ps1"

Write-Host "== Validate truth alignment ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check_truth_alignment.ps1"

Write-Host "== Validate template claims ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check_template_claims.ps1"

Write-Host "== Validate subsystem badges ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check_subsystem_badges.ps1"

Write-Host "== Configure: $ConfigurePreset ==" -ForegroundColor Cyan
cmake --preset $ConfigurePreset

if (-not $SkipBuild) {
    Write-Host "== Build: $BuildPreset ==" -ForegroundColor Cyan
    cmake --build --preset $BuildPreset
}

Write-Host "== Validate visual regression harness ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_visual_regression_harness.ps1"

Write-Host "== Validate localization consistency ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_localization_consistency.ps1"

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
