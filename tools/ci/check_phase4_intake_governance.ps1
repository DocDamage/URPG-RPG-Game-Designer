$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Set-Location $repoRoot

$requiredFiles = @(
    "docs/external-intake/repo-watchlist.md",
    "docs/external-intake/license-matrix.md",
    "docs/external-intake/repo-audit-template.md",
    "docs/external-intake/urpg_feature_adoption_matrix.md",
    "docs/external-intake/asset-attribution.schema.json",
    "docs/external-intake/plugin-fixture-metadata.schema.json",
    "docs/external-intake/reference-note-template.md",
    "docs/asset_intake/ASSET_SOURCE_REGISTRY.md",
    "docs/asset_intake/ASSET_PROMOTION_GUIDE.md",
    "docs/asset_intake/ASSET_CATEGORY_GAPS.md",
    "imports/manifests/asset_sources/asset_source.schema.json",
    "imports/manifests/asset_bundles/asset_bundle.schema.json",
    "imports/manifests/asset_sources/SRC-001.json",
    "imports/manifests/asset_sources/SRC-002.json",
    "imports/manifests/asset_sources/SRC-003.json",
    "imports/manifests/asset_sources/SRC-004.json",
    "imports/manifests/asset_sources/SRC-005.json",
    "imports/manifests/asset_bundles/BND-001.json",
    "imports/manifests/asset_bundles/BND-002.json",
    "imports/normalized/prototype_sprites/gdquest_blue_actor.svg",
    "imports/normalized/ui_sfx/kenney_click_001.wav",
    "imports/reports/asset_intake/source_capture_status.json",
    "imports/reports/asset_intake/wysiwyg_smoke_proof.json",
    "third_party/external-repos/README.md",
    "imports/fixtures/compat/README.md",
    "imports/fixtures/legacy-projects/README.md",
    "imports/fixtures/localization/README.md",
    "imports/fixtures/assets/README.md"
)

$missing = @()
foreach ($path in $requiredFiles) {
    if (-not (Test-Path $path)) {
        $missing += $path
    }
}

if ($missing.Count -gt 0) {
    $missing | ForEach-Object { Write-Host "Missing required Phase 4 artifact: $_" }
    exit 1
}

$placeholderFiles = @(
    "docs/external-intake/repo-watchlist.md",
    "docs/external-intake/license-matrix.md",
    "docs/external-intake/urpg_feature_adoption_matrix.md",
    "docs/asset_intake/ASSET_SOURCE_REGISTRY.md",
    "docs/asset_intake/ASSET_CATEGORY_GAPS.md",
    "docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md",
    "docs/PROGRAM_COMPLETION_STATUS.md",
    "WORKLOG.md",
    "docs/presentation/PLAN.md"
)

$placeholderPatterns = @(
    "Pending audit",
    "\bTBD\b"
)

$hasError = $false
foreach ($path in $placeholderFiles) {
    $raw = Get-Content $path -Raw
    foreach ($pattern in $placeholderPatterns) {
        if ($raw -match $pattern) {
            Write-Host "Placeholder pattern '$pattern' still present in $path"
            $hasError = $true
        }
    }
}

$expectedSourceIds = @("SRC-001", "SRC-002", "SRC-003", "SRC-004", "SRC-005")
foreach ($sourceId in $expectedSourceIds) {
    $manifestPath = "imports/manifests/asset_sources/$sourceId.json"
    try {
        $manifest = Get-Content $manifestPath -Raw | ConvertFrom-Json
    } catch {
        Write-Host "Invalid JSON manifest: $manifestPath"
        $hasError = $true
        continue
    }

    if ($manifest.source_id -ne $sourceId) {
        Write-Host "Manifest source_id mismatch in $manifestPath"
        $hasError = $true
    }
}

try {
    $assetReport = Get-Content "imports/reports/asset_intake/source_capture_status.json" -Raw | ConvertFrom-Json
} catch {
    Write-Host "Invalid asset intake status report."
    $hasError = $true
}

if ($null -ne $assetReport) {
    if ($assetReport.summary.normalized -lt 1 -or $assetReport.summary.promoted -lt 1) {
        Write-Host "Asset intake report must include at least one normalized and one promoted asset."
        $hasError = $true
    }
    if ($assetReport.summary.promoted_visual_lanes -lt 1 -or $assetReport.summary.promoted_audio_lanes -lt 1) {
        Write-Host "Asset intake report must include promoted visual and audio lanes."
        $hasError = $true
    }
    if ($assetReport.summary.wysiwyg_smoke_proofs -lt 1) {
        Write-Host "Asset intake report must include a WYSIWYG smoke proof."
        $hasError = $true
    }
}

$licenseMatrix = Get-Content "docs/external-intake/license-matrix.md" -Raw
$watchlist = Get-Content "docs/external-intake/repo-watchlist.md" -Raw
if (($watchlist | Select-String -Pattern "https://github.com/" -AllMatches).Matches.Count -ne 12) {
    Write-Host "Repo watchlist does not contain 12 GitHub repo entries."
    $hasError = $true
}
if (($licenseMatrix | Select-String -Pattern "\| .*?/.*? \|" -AllMatches).Matches.Count -lt 12) {
    Write-Host "License matrix does not appear to contain all 12 intake repos."
    $hasError = $true
}
if ($licenseMatrix -notmatch "wrapper" -or $licenseMatrix -notmatch "facade") {
    Write-Host "External intake governance does not explicitly require wrappers/facades for production-candidate adoption."
    $hasError = $true
}

$assetPromotionGuide = Get-Content "docs/asset_intake/ASSET_PROMOTION_GUIDE.md" -Raw
if ($assetPromotionGuide -notmatch "promotion record" -or
    $assetPromotionGuide -notmatch "source manifest" -or
    $assetPromotionGuide -notmatch "bundle manifest") {
    Write-Host "Asset promotion guide does not require provenance-preserving promotion records."
    $hasError = $true
}

$remediation = Get-Content "docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md" -Raw
if ($remediation -notmatch "\| P3-02 \| External Repository Intake Needs Canonical Governance \| ✅ Remediated \|") {
    Write-Host "P3-02 is not marked remediated in the technical debt remediation plan."
    $hasError = $true
}
if ($remediation -notmatch "\| P3-03 \| Private-Use Asset Intake Needs Canonical Governance \| ✅ Remediated \|") {
    Write-Host "P3-03 is not marked remediated in the technical debt remediation plan."
    $hasError = $true
}

$programStatus = Get-Content "docs/PROGRAM_COMPLETION_STATUS.md" -Raw
if ($programStatus -notmatch "Phase 4 governance/reconciliation closure is complete" -or
    $programStatus -notmatch "Phase 5 hardening closure is also complete") {
    Write-Host "Program completion status does not reflect the closed Phase 4 and Phase 5 governance state."
    $hasError = $true
}

if ($hasError) {
    exit 1
}

Write-Host "Phase 4 intake governance validation passed."
