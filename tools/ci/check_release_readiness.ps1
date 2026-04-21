$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$readinessPath = Join-Path $repoRoot "content\readiness\readiness_status.json"
$matrixPath = Join-Path $repoRoot "docs\RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs\TEMPLATE_READINESS_MATRIX.md"

if (-not (Test-Path $readinessPath)) {
    throw "Missing readiness dataset: $readinessPath"
}
if (-not (Test-Path $matrixPath)) {
    throw "Missing release readiness matrix: $matrixPath"
}
if (-not (Test-Path $templateMatrixPath)) {
    throw "Missing template readiness matrix: $templateMatrixPath"
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$tick = [string][char]96

if (-not $readiness.schemaVersion) {
    throw "Readiness dataset must include schemaVersion."
}

$allowedStatuses = @("READY", "PARTIAL", "EXPERIMENTAL", "BLOCKED", "PLANNED")

foreach ($entry in $readiness.subsystems) {
    if ($allowedStatuses -notcontains $entry.status) {
        throw "Subsystem '$($entry.id)' has invalid status '$($entry.status)'."
    }
}

foreach ($entry in $readiness.templates) {
    if ($allowedStatuses -notcontains $entry.status) {
        throw "Template '$($entry.id)' has invalid status '$($entry.status)'."
    }
}

$matrixText = Get-Content -Raw -Path $matrixPath
$templateMatrixText = Get-Content -Raw -Path $templateMatrixPath

foreach ($entry in $readiness.subsystems) {
    $pattern = [regex]::Escape($tick + $entry.id + $tick)
    if ($matrixText -notmatch $pattern) {
        throw "Release readiness matrix is missing subsystem row '$($entry.id)'."
    }
}

foreach ($entry in $readiness.templates) {
    $pattern = [regex]::Escape($tick + $entry.id + $tick)
    if ($templateMatrixText -notmatch $pattern) {
        throw "Template readiness matrix is missing template row '$($entry.id)'."
    }
}

# Verify all READY subsystems have fully true evidence
foreach ($entry in $readiness.subsystems) {
    if ($entry.status -eq "READY") {
        $evidenceFields = @("runtimeOwner", "editorSurface", "schemaMigration", "diagnostics", "testsValidation", "docsAligned")
        foreach ($field in $evidenceFields) {
            if ($entry.evidence.$field -ne $true) {
                throw "Subsystem '$($entry.id)' is marked READY but evidence field '$field' is not true."
            }
        }
    }
}

# Build lookup of known subsystem IDs
$knownSubsystemIds = @{}
foreach ($entry in $readiness.subsystems) {
    $knownSubsystemIds[$entry.id] = $true
}

# Verify READY and PARTIAL templates reference only known subsystems
foreach ($entry in $readiness.templates) {
    if ($entry.status -eq "READY" -or $entry.status -eq "PARTIAL") {
        foreach ($requiredId in $entry.requiredSubsystems) {
            if (-not $knownSubsystemIds.ContainsKey($requiredId)) {
                throw "Template '$($entry.id)' requires unknown subsystem '$requiredId'."
            }
        }
    }
}

Write-Host "Release readiness records and matrices are present and minimally aligned."
Write-Host "All READY subsystem evidence fields are true."
Write-Host "All READY/PARTIAL template required subsystems are known."
