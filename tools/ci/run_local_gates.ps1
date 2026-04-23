param(
    [string]$ConfigurePreset,
    [string]$BuildPreset,
    [string]$PresentationConfiguration,
    [switch]$SkipBuild,
    [switch]$SkipPresentationGate,
    [switch]$SkipWarningsAsErrorsGate
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

Write-Host "== Validate truth reconciliation ==" -ForegroundColor Cyan
& "$PSScriptRoot\truth_reconciler.ps1"

Write-Host "== Validate template spec bar drift ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_template_spec_bar_drift.ps1"

Write-Host "== Validate tooling boundary (engine must not import tool code) ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_tooling_boundary.ps1"

Write-Host "== Validate schema changelog governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_schema_changelog.ps1"

Write-Host "== Validate save policy governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_save_policy_governance.ps1"

Write-Host "== Validate accessibility governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_accessibility_governance.ps1"

Write-Host "== Validate audio governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_audio_governance.ps1"

Write-Host "== Validate input governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_input_governance.ps1"

Write-Host "== Validate achievement governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_achievement_governance.ps1"

Write-Host "== Validate character governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_character_governance.ps1"

Write-Host "== Validate mod governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_mod_governance.ps1"

Write-Host "== Validate analytics governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_analytics_governance.ps1"

Write-Host "== Compat corpus health check ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_compat_health.ps1"

Write-Host "== Validate truth alignment ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check_truth_alignment.ps1"

Write-Host "== Validate template claims ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check_template_claims.ps1"

Write-Host "== Validate subsystem badges ==" -ForegroundColor Cyan
& "$PSScriptRoot\..\docs\check_subsystem_badges.ps1"

Write-Host "== Validate breaking-change governance ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_breaking_changes.ps1"

Write-Host "== Validate CMake completeness ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_cmake_completeness.ps1"

Write-Host "== Configure: $ConfigurePreset ==" -ForegroundColor Cyan
cmake --preset $ConfigurePreset

if (-not $SkipBuild) {
    Write-Host "== Build: $BuildPreset ==" -ForegroundColor Cyan
    cmake --build --preset $BuildPreset
}

if (-not $SkipWarningsAsErrorsGate) {
    $strictBuildDir = "build/$ConfigurePreset-warnings-as-errors"
    Write-Host "== Configure strict warnings gate: $strictBuildDir ==" -ForegroundColor Cyan
    cmake --preset $ConfigurePreset -B $strictBuildDir -DURPG_WARNINGS_AS_ERRORS=ON

    Write-Host "== Build strict warnings gate ==" -ForegroundColor Cyan
    cmake --build $strictBuildDir --target `
        urpg_migrate `
        urpg_project_audit `
        urpg_export_smoke_app `
        urpg_presentation_release_validation `
        urpg_tests `
        urpg_integration_tests `
        urpg_snapshot_tests `
        urpg_compat_tests
}

$testDir = "build/$ConfigurePreset"

Write-Host "== Validate visual regression harness ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_visual_regression_harness.ps1"

Write-Host "== Validate renderer-backed visual capture ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_renderer_backed_visual_capture.ps1" -BuildDirectory $testDir

Write-Host "== Validate localization consistency ==" -ForegroundColor Cyan
& "$PSScriptRoot\check_localization_consistency.ps1"

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
ctest --test-dir $testDir --output-on-failure -L "^pr$"

Write-Host "== Gate 2 (nightly) ==" -ForegroundColor Cyan
ctest --test-dir $testDir --output-on-failure -L "^nightly$"

Write-Host "== Gate 3 (weekly) ==" -ForegroundColor Cyan
ctest --test-dir $testDir --output-on-failure -L "^weekly$"

Write-Host "All gates passed." -ForegroundColor Green
