$ErrorActionPreference = "Stop"

$ReportPath = "imports/reports/asset_intake/lfs_release_scope_report.json"

$lfs = @(git lfs ls-files --name-only | Where-Object { $_ -ne "" })
$releaseRequired = Get-Content content/fixtures/project_governance_fixture.json -Raw | ConvertFrom-Json
$requiredPaths = @()
foreach ($asset in $releaseRequired.releaseAssets.assets) {
    if ($asset.path) {
        $requiredPaths += $asset.path
    }
}

$releaseLfs = @($lfs | Where-Object { $requiredPaths -contains $_ })
if ($releaseLfs.Count -gt 0) {
    Write-Error ("Release-required assets are still LFS-tracked:`n" + ($releaseLfs | Out-String))
}

$knownScopes = @(
    @{
        Root = "imports/normalized/sibling_bulk_assets/"
        Scope = "deferred_curated_library"
        SourceId = "SRC-015"
        BundleId = "BND-011"
        ClaimBoundary = "release-eligible library payload; not release-required; exported only when a project selects the asset"
    },
    @{
        Root = "imports/normalized/present_raw_local_bulk/"
        Scope = "deferred_curated_library"
        SourceId = "SRC-014"
        BundleId = "BND-010"
        ClaimBoundary = "release-eligible library payload; not release-required; exported only when a project selects the asset"
    },
    @{
        Root = "imports/normalized/src010_cc0_release_bulk/"
        Scope = "deferred_curated_library"
        SourceId = "SRC-010"
        BundleId = "BND-006"
        ClaimBoundary = "release-eligible library payload; not release-required; exported only when a project selects the asset"
    },
    @{
        Root = "imports/normalized/src010_newly_licensed_bulk/"
        Scope = "deferred_curated_library"
        SourceId = "SRC-010"
        BundleId = "BND-007"
        ClaimBoundary = "release-eligible library payload; not release-required; exported only when a project selects the asset"
    },
    @{
        Root = "imports/normalized/itch_loose_cc0/"
        Scope = "deferred_curated_library"
        SourceId = "SRC-013"
        BundleId = "BND-008"
        ClaimBoundary = "release-eligible library payload; not release-required; exported only when a project selects the asset"
    },
    @{
        Root = "imports/manifests/asset_bundles/"
        Scope = "governance_metadata"
        SourceId = ""
        BundleId = ""
        ClaimBoundary = "large bundle manifest metadata; not runtime asset payload"
    },
    @{
        Root = "imports/reports/asset_intake/"
        Scope = "governance_report"
        SourceId = ""
        BundleId = ""
        ClaimBoundary = "large generated report evidence; not runtime asset payload"
    }
)

$scopeRows = @()
$unknown = New-Object System.Collections.Generic.List[string]

foreach ($item in $knownScopes) {
    $matches = @($lfs | Where-Object { $_.StartsWith($item.Root, [System.StringComparison]::Ordinal) })
    $scopeRows += [ordered]@{
        root = $item.Root
        scope = $item.Scope
        source_id = $item.SourceId
        bundle_id = $item.BundleId
        count = $matches.Count
        claim_boundary = $item.ClaimBoundary
    }
}

foreach ($path in $lfs) {
    $matched = $false
    foreach ($item in $knownScopes) {
        if ($path.StartsWith($item.Root, [System.StringComparison]::Ordinal)) {
            $matched = $true
            break
        }
    }
    if (-not $matched) {
        $unknown.Add($path)
    }
}

$report = [ordered]@{
    generated_at_utc = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
    total_lfs_paths = $lfs.Count
    release_required_lfs_paths = $releaseLfs
    release_required_lfs_count = $releaseLfs.Count
    unknown_lfs_count = $unknown.Count
    unknown_lfs_sample = @($unknown | Select-Object -First 50)
    scopes = $scopeRows
    claim_boundary = "Only release-required app assets are independently verified by the release asset gate. Deferred curated-library LFS payloads are governed inventory, not proof of final art/content completeness or zero-LFS repository status."
}

$reportDir = Split-Path $ReportPath -Parent
if (-not (Test-Path $reportDir)) {
    New-Item -ItemType Directory -Path $reportDir | Out-Null
}
$report | ConvertTo-Json -Depth 6 | Set-Content -Path $ReportPath -Encoding UTF8

if ($unknown.Count -gt 0) {
    Write-Error ("Unknown LFS-tracked paths are outside the governed release-scope allowlist. Sample:`n" + (($unknown | Select-Object -First 50) | Out-String))
}

Write-Host "Release-required assets are not LFS-tracked. Total LFS paths in checkout: $($lfs.Count)"
Write-Host "LFS release-scope report: $ReportPath"
