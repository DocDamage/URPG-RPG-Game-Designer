$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$schemasDir = Join-Path $repoRoot "content/schemas"
$changelogPath = Join-Path $repoRoot "docs/SCHEMA_CHANGELOG.md"
$referenceDate = Get-Date -Year 2026 -Month 4 -Day 20
$cutoffDate = $referenceDate.AddDays(-90)

$hadWarning = $false

Write-Host "Checking schema breaking-change governance..."

# 1. Verify readiness_status.schema.json exists
$readinessSchemaPath = Join-Path $schemasDir "readiness_status.schema.json"
if (-not (Test-Path $readinessSchemaPath)) {
    throw "Missing required schema: readiness_status.schema.json"
}
Write-Host "PASS: readiness_status.schema.json exists."

# 2. Verify SCHEMA_CHANGELOG.md exists
if (-not (Test-Path $changelogPath)) {
    throw "Missing schema changelog: $changelogPath"
}
Write-Host "PASS: SCHEMA_CHANGELOG.md exists."

# 3. Verify changelog has entries dated within the last 90 days
$changelogText = Get-Content -Raw -Path $changelogPath
$dateMatches = [regex]::Matches($changelogText, '\d{4}-\d{2}-\d{2}')
$recentEntryFound = $false
foreach ($match in $dateMatches) {
    $entryDate = [datetime]::ParseExact($match.Value, "yyyy-MM-dd", $null)
    if ($entryDate -ge $cutoffDate) {
        $recentEntryFound = $true
        break
    }
}
if (-not $recentEntryFound) {
    throw "SCHEMA_CHANGELOG.md has no entries newer than $($cutoffDate.ToString('yyyy-MM-dd'))."
}
Write-Host "PASS: SCHEMA_CHANGELOG.md contains entries within the last 90 days."

# 4. Check every .schema.json has $id or title, and warn if not mentioned in changelog
$schemaFiles = Get-ChildItem -Path $schemasDir -Filter "*.schema.json"
if ($schemaFiles.Count -eq 0) {
    throw "No schema files found in $schemasDir"
}

foreach ($schemaFile in $schemaFiles) {
    $schemaJson = Get-Content -Raw -Path $schemaFile.FullName | ConvertFrom-Json
    $hasId = [bool]($schemaJson.PSObject.Properties["`$id"])
    $hasTitle = [bool]($schemaJson.PSObject.Properties["title"])

    if (-not $hasId -and -not $hasTitle) {
        throw "Schema '$($schemaFile.Name)' is missing both '`$id' and 'title'."
    }

    $fileName = $schemaFile.Name
    if ($changelogText -notmatch [regex]::Escape($fileName)) {
        Write-Host "WARNING: Schema '$fileName' is not mentioned in SCHEMA_CHANGELOG.md." -ForegroundColor Yellow
        $hadWarning = $true
    }
}

Write-Host "PASS: All $($schemaFiles.Count) schema files have '`$id' or 'title'."

if ($hadWarning) {
    Write-Host "Completed with warnings. Review missing changelog mentions above." -ForegroundColor Yellow
}
else {
    Write-Host "All breaking-change checks passed."
}
