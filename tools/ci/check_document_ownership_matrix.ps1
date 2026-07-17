param(
    [string]$MatrixPath = "content/readiness/document_ownership_matrix.json"
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$resolvedMatrix = Join-Path $repoRoot $MatrixPath
if (-not (Test-Path -LiteralPath $resolvedMatrix -PathType Leaf)) {
    throw "Document ownership matrix is missing: $resolvedMatrix"
}

$matrix = Get-Content -LiteralPath $resolvedMatrix -Raw | ConvertFrom-Json
if ($matrix.schema -ne "urpg.document_ownership_matrix.v1") {
    throw "Document ownership matrix schema is invalid."
}

$expectedIds = @(
    "map.grid_parts", "map.perspective_2d", "ability.draft", "character.creator", "quest.draft",
    "dialogue.draft", "database.project", "vendor.catalog", "audio.mix", "input.remap", "input.controller_bindings",
    "gameplay.recipes", "menu.studio", "compat.mz_plugin_lock"
)
$documents = @($matrix.documents)
if ($documents.Count -ne $expectedIds.Count) {
    throw "Document ownership matrix must contain exactly $($expectedIds.Count) registered durable documents; found $($documents.Count)."
}

$mainSource = Get-Content -LiteralPath (Join-Path $repoRoot "apps/editor/main.cpp") -Raw
$seen = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
$allowedAtomic = @("implemented", "unproven")
$allowedRecovery = @("implemented_private_overlay")
foreach ($document in $documents) {
    $id = [string]$document.dirtyDocumentId
    if (-not $seen.Add($id)) { throw "Duplicate document ownership row: $id" }
    if ($expectedIds -notcontains $id) { throw "Unknown durable document ownership row: $id" }
    foreach ($field in @("documentType", "owner", "pathPattern", "saveOwner", "historyStatus", "migrationStatus", "referenceStatus", "externalChangeStatus")) {
        if ([string]::IsNullOrWhiteSpace([string]$document.$field)) { throw "${id}: missing $field." }
    }
    if ($allowedAtomic -notcontains [string]$document.atomicSaveStatus) { throw "${id}: invalid atomicSaveStatus." }
    if ($allowedRecovery -notcontains [string]$document.recoveryStatus) { throw "${id}: invalid recoveryStatus." }
    if ([string]$document.externalChangeStatus -notin @("implemented_live_compare_reload_keep_local", "missing_live_registration")) {
        throw "${id}: invalid externalChangeStatus."
    }
    if (@($document.gaps).Count -eq 0) { throw "${id}: gaps must be explicit, even when empty capability breadth is intentional." }
    foreach ($ownerFile in @($document.ownerFiles)) {
        if (-not (Test-Path -LiteralPath (Join-Path $repoRoot ([string]$ownerFile)) -PathType Leaf)) {
            throw "${id}: owner evidence file is missing: $ownerFile"
        }
    }
    if (-not $mainSource.Contains('"' + $id + '"')) {
        throw "${id}: apps/editor/main.cpp does not contain the claimed dirty/recovery document ID."
    }
}

$missing = @($expectedIds | Where-Object { -not $seen.Contains($_) })
if ($missing.Count -gt 0) { throw "Missing durable document ownership rows: $($missing -join ', ')" }

$liveExternalChanges = @($documents | Where-Object externalChangeStatus -eq "implemented_live_compare_reload_keep_local").Count
Write-Host "Document ownership matrix passed: $($documents.Count) durable native documents have explicit owner, persistence, history, migration, reference, external-change, recovery, and gap status; live external-change owners=$liveExternalChanges."
