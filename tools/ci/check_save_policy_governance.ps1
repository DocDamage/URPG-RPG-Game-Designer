#Requires -Version 5.1
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$fixturePath = Join-Path $repoRoot "content/fixtures/save_policies.json"
$schemaPath = Join-Path $repoRoot "content/schemas/save_policies.schema.json"
$changelogPath = Join-Path $repoRoot "docs/SCHEMA_CHANGELOG.md"

# 1. Fixture exists
if (-not (Test-Path $fixturePath)) {
    throw "Missing canonical save policy fixture: content/fixtures/save_policies.json"
}

# 2. Valid JSON
try {
    $policy = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
} catch {
    throw "Save policy fixture is not valid JSON"
}

# 3. Required keys
$requiredKeys = @("_urpg_format_version", "metadata_fields", "retention", "autosave")
foreach ($key in $requiredKeys) {
    if (-not ($policy.PSObject.Properties.Name -contains $key)) {
        throw "Save policy fixture missing required key: $key"
    }
}

# 4. Retention limits non-negative
$retentionLimits = @(
    @("max_autosave_slots", $policy.retention.max_autosave_slots),
    @("max_quicksave_slots", $policy.retention.max_quicksave_slots),
    @("max_manual_slots", $policy.retention.max_manual_slots)
)
foreach ($limit in $retentionLimits) {
    $field = $limit[0]
    $value = $limit[1]
    if ($value -lt 0) {
        throw "Save policy retention limit $field cannot be negative: $value"
    }
}

# 5. Autosave slot ID non-negative
if ($policy.autosave.slot_id -lt 0) {
    throw "Save policy autosave.slot_id cannot be negative: $($policy.autosave.slot_id)"
}

# 6. Schema changelog entry
$changelogText = Get-Content -Raw -Path $changelogPath
if ($changelogText -notmatch "save_policies\.schema\.json") {
    throw "Schema changelog missing entry for save_policies.schema.json"
}

Write-Host "Save policy governance validation passed."
