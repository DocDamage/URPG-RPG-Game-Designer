param(
    [string]$ConfigurePreset,
    [string]$BuildPreset,
    [string]$PresentationConfiguration,
    [int]$CTestTimeoutSeconds = 300,
    [switch]$SkipBuild,
    [switch]$SkipPresentationGate,
    [switch]$SkipWarningsAsErrorsGate,
    [switch]$RunReleaseCandidateGate,
    [string]$ReleaseCandidateLfsWaiverReference = ""
)

$ErrorActionPreference = "Stop"
. "$PSScriptRoot\resolve-local-cmake-profile.ps1"

function Assert-LastExitCode {
    param(
        [string]$StepName
    )

    if ($LASTEXITCODE -ne 0) {
        throw "$StepName failed with exit code $LASTEXITCODE."
    }
}

function Get-AbsoluteRepoPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }

    return Join-Path $repoRoot $Path
}

function Repair-NinjaLog {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BuildDirectory
    )

    foreach ($metadataFileName in @(".ninja_log", ".ninja_deps")) {
        $metadataPath = Join-Path $BuildDirectory $metadataFileName
        if (Test-Path $metadataPath) {
            Remove-Item -LiteralPath $metadataPath -Force
            Write-Host "Removed stale Ninja metadata: $metadataPath" -ForegroundColor DarkGray
        }
    }
}

function New-BuildTreeLock {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BuildDirectory
    )

    New-Item -ItemType Directory -Path $BuildDirectory -Force | Out-Null
    $lockPath = Join-Path $BuildDirectory ".urpg-local-gates.lock"
    if (Test-Path $lockPath) {
        $lock = Get-Item $lockPath
        if ($lock.LastWriteTime -lt (Get-Date).AddHours(-12)) {
            Remove-Item -LiteralPath $lockPath -Force
        } else {
            throw "Local gate lock already exists at '$lockPath'. Another gate run may be using this build tree. Remove the lock only after confirming no gate/build is active."
        }
    }

    $stream = [System.IO.File]::Open(
        $lockPath,
        [System.IO.FileMode]::CreateNew,
        [System.IO.FileAccess]::Write,
        [System.IO.FileShare]::None
    )
    $writer = [System.IO.StreamWriter]::new($stream)
    $writer.WriteLine("pid=$PID")
    $writer.WriteLine("started=$(Get-Date -Format o)")
    $writer.Flush()

    return [pscustomobject]@{
        Path = $lockPath
        Writer = $writer
        Stream = $stream
    }
}

function Remove-BuildTreeLock {
    param(
        [Parameter(Mandatory = $true)]
        $Lock
    )

    if ($null -ne $Lock.Writer) {
        $Lock.Writer.Dispose()
    } elseif ($null -ne $Lock.Stream) {
        $Lock.Stream.Dispose()
    }

    if (Test-Path $Lock.Path) {
        Remove-Item -LiteralPath $Lock.Path -Force
    }
}

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

$testDir = "build/$ConfigurePreset"
$absoluteTestDir = Get-AbsoluteRepoPath $testDir
$strictBuildDir = "build/$ConfigurePreset-warnings-as-errors"
$absoluteStrictBuildDir = Get-AbsoluteRepoPath $strictBuildDir

$gateLock = New-BuildTreeLock -BuildDirectory $absoluteTestDir

try {
    Repair-NinjaLog -BuildDirectory $absoluteTestDir
    if (-not $SkipWarningsAsErrorsGate) {
        Repair-NinjaLog -BuildDirectory $absoluteStrictBuildDir
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
    Assert-LastExitCode "Configure preset '$ConfigurePreset'"

    if (-not $SkipBuild) {
        Write-Host "== Build: $BuildPreset ==" -ForegroundColor Cyan
        cmake --build --preset $BuildPreset
        Assert-LastExitCode "Build preset '$BuildPreset'"
    }

    Write-Host "== Validate platform export matrix ==" -ForegroundColor Cyan
    & "$PSScriptRoot\check_platform_exports.ps1" -BuildDirectory $testDir

    Write-Host "== Validate release packaging dry run ==" -ForegroundColor Cyan
    & "$PSScriptRoot\package_release_artifacts.ps1" -Mode DevUnsigned -BuildDirectory $testDir -DryRun

    if (-not $SkipWarningsAsErrorsGate) {
        Write-Host "== Configure strict warnings gate: $strictBuildDir ==" -ForegroundColor Cyan
        cmake --preset $ConfigurePreset -B $strictBuildDir -DURPG_WARNINGS_AS_ERRORS=ON
        Assert-LastExitCode "Configure strict warnings gate '$strictBuildDir'"

        Write-Host "== Build strict warnings gate ==" -ForegroundColor Cyan
        cmake --build $strictBuildDir --target `
            urpg_migrate `
            urpg_project_audit `
            urpg_export_smoke_app `
            urpg_presentation_release_validation `
            urpg_tests `
            urpg_render_tests `
            urpg_integration_tests `
            urpg_snapshot_canonical_tests `
            urpg_snapshot_renderer_tests `
            urpg_compat_tests
        Assert-LastExitCode "Build strict warnings gate"
    }

    Write-Host "== Validate CTest discovery ==" -ForegroundColor Cyan
    & "$PSScriptRoot\check_ctest_discovery.ps1" -BuildDirectory $testDir

    Write-Host "== Validate visual regression harness ==" -ForegroundColor Cyan
    & "$PSScriptRoot\check_visual_regression_harness.ps1"

    Write-Host "== Validate visual golden governance ==" -ForegroundColor Cyan
    & "$PSScriptRoot\check_visual_golden_governance.ps1"

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
        -PluginRoot "imports\raw\third_party_assets\rpgmaker-mz\steam-dlc\plugin-dropins-curated\js\plugins" `
        -ReportPrefix "plugin_dropins_curated_ci_validation" `
        -FailOnError

    Write-Host "== Gate 1 (pr) ==" -ForegroundColor Cyan
    ctest --test-dir $testDir --output-on-failure --timeout $CTestTimeoutSeconds -L "^pr$"
    Assert-LastExitCode "Gate 1 (pr)"

    Write-Host "== Gate 2 (nightly) ==" -ForegroundColor Cyan
    ctest --test-dir $testDir --output-on-failure --timeout $CTestTimeoutSeconds -L "^nightly$"
    Assert-LastExitCode "Gate 2 (nightly)"

    Write-Host "== Gate 3 (weekly) ==" -ForegroundColor Cyan
    ctest --test-dir $testDir --output-on-failure --timeout $CTestTimeoutSeconds -L "^weekly$"
    Assert-LastExitCode "Gate 3 (weekly)"

    if ($RunReleaseCandidateGate) {
        Write-Host "== Release candidate gate ==" -ForegroundColor Cyan
        $releaseCandidateArgs = @(
            "-ConfigurePreset", $ConfigurePreset,
            "-BuildPreset", $BuildPreset,
            "-BuildDirectory", $testDir,
            "-Configuration", $PresentationConfiguration,
            "-CTestTimeoutSeconds", $CTestTimeoutSeconds,
            "-SkipConfigure",
            "-SkipBuild"
        )
        if (-not [string]::IsNullOrWhiteSpace($ReleaseCandidateLfsWaiverReference)) {
            $releaseCandidateArgs += @(
                "-SkipLfsHydration",
                "-LfsWaiverReference", $ReleaseCandidateLfsWaiverReference
            )
        }
        & "$PSScriptRoot\run_release_candidate_gate.ps1" @releaseCandidateArgs
    }

    Write-Host "All gates passed." -ForegroundColor Green
} finally {
    Remove-BuildTreeLock -Lock $gateLock
}
