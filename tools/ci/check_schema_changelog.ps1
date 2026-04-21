$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$schemaPath = Join-Path $repoRoot "content\schemas\readiness_status.schema.json"
$dataPath = Join-Path $repoRoot "content\readiness\readiness_status.json"
$changelogPath = Join-Path $repoRoot "docs\SCHEMA_CHANGELOG.md"

foreach ($file in @($schemaPath, $dataPath, $changelogPath)) {
    if (-not (Test-Path $file)) {
        throw "Missing schema-governance file: $file"
    }
}

$schemaJson = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
$dataJson = Get-Content -Raw -Path $dataPath | ConvertFrom-Json
$changelogText = Get-Content -Raw -Path $changelogPath

if (-not $schemaJson.properties.schemaVersion) {
    throw "Readiness schema must define a schemaVersion field."
}

if (-not $dataJson.schemaVersion) {
    throw "Readiness dataset must declare schemaVersion."
}

if ($changelogText -notmatch "readiness_status") {
    throw "Schema changelog must include an entry for readiness_status."
}

if ($changelogText -notmatch [regex]::Escape($dataJson.schemaVersion)) {
    throw "Schema changelog must mention readiness dataset version '$($dataJson.schemaVersion)'."
}

Write-Host "Schema changelog governance checks passed."
