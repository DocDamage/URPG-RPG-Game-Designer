$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$readinessPath = Join-Path $repoRoot "content\readiness\readiness_status.json"
$matrixPath = Join-Path $repoRoot "docs\RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs\TEMPLATE_READINESS_MATRIX.md"
$truthRulesPath = Join-Path $repoRoot "docs\TRUTH_ALIGNMENT_RULES.md"

if (-not (Test-Path $readinessPath)) {
    throw "Missing readiness dataset: $readinessPath"
}
if (-not (Test-Path $matrixPath)) {
    throw "Missing release readiness matrix: $matrixPath"
}
if (-not (Test-Path $templateMatrixPath)) {
    throw "Missing template readiness matrix: $templateMatrixPath"
}
if (-not (Test-Path $truthRulesPath)) {
    throw "Missing truth alignment rules: $truthRulesPath"
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$tick = [string][char]96

function Get-StatusDateFromText {
    param(
        [string]$Text,
        [string]$Label
    )

    $match = [regex]::Match($Text, 'Status Date:\s*(\d{4}-\d{2}-\d{2})')
    if (-not $match.Success) {
        throw "$Label must declare a Status Date."
    }
    return $match.Groups[1].Value
}

function Get-MatrixRows {
    param(
        [string]$Text,
        [string]$Kind
    )

    $rows = @{}
    $seenCurrentMatrix = $false
    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match '^## Current Matrix') {
            $seenCurrentMatrix = $true
            continue
        }
        if (-not $seenCurrentMatrix) {
            continue
        }
        if ($line -match '^## ') {
            break
        }
        if ($line -match '^\| `([^`]+)` \| `([^`]+)` \|') {
            $rows[$matches[1]] = $matches[2]
        }
    }

    if ($rows.Count -eq 0) {
        throw "Could not parse any $Kind matrix rows from the current matrix section."
    }

    return $rows
}

if (-not $readiness.schemaVersion) {
    throw "Readiness dataset must include schemaVersion."
}
if (-not $readiness.statusDate) {
    throw "Readiness dataset must include statusDate."
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
$truthRulesText = Get-Content -Raw -Path $truthRulesPath

$releaseMatrixDate = Get-StatusDateFromText -Text $matrixText -Label "Release readiness matrix"
$templateMatrixDate = Get-StatusDateFromText -Text $templateMatrixText -Label "Template readiness matrix"
$truthRulesDate = Get-StatusDateFromText -Text $truthRulesText -Label "Truth alignment rules"

foreach ($docDate in @(
        @{ Label = "Release readiness matrix"; Value = $releaseMatrixDate },
        @{ Label = "Template readiness matrix"; Value = $templateMatrixDate },
        @{ Label = "Truth alignment rules"; Value = $truthRulesDate }
    )) {
    if ($docDate.Value -ne $readiness.statusDate) {
        throw "$($docDate.Label) status date '$($docDate.Value)' does not match readiness_status.json date '$($readiness.statusDate)'."
    }
}

$releaseRows = Get-MatrixRows -Text $matrixText -Kind "release"
$templateRows = Get-MatrixRows -Text $templateMatrixText -Kind "template"

foreach ($entry in $readiness.subsystems) {
    if (-not $releaseRows.ContainsKey($entry.id)) {
        throw "Release readiness matrix is missing subsystem row '$($entry.id)'."
    }
    if ($releaseRows[$entry.id] -ne $entry.status) {
        throw "Release readiness matrix status for subsystem '$($entry.id)' is '$($releaseRows[$entry.id])' but readiness_status.json says '$($entry.status)'."
    }
}

foreach ($entry in $readiness.templates) {
    if (-not $templateRows.ContainsKey($entry.id)) {
        throw "Template readiness matrix is missing template row '$($entry.id)'."
    }
    if ($templateRows[$entry.id] -ne $entry.status) {
        throw "Template readiness matrix status for template '$($entry.id)' is '$($templateRows[$entry.id])' but readiness_status.json says '$($entry.status)'."
    }
}

foreach ($matrixId in $releaseRows.Keys) {
    if (-not ($readiness.subsystems | Where-Object { $_.id -eq $matrixId })) {
        throw "Release readiness matrix contains subsystem row '$matrixId' that is missing from readiness_status.json."
    }
}

foreach ($matrixId in $templateRows.Keys) {
    if (-not ($readiness.templates | Where-Object { $_.id -eq $matrixId })) {
        throw "Template readiness matrix contains template row '$matrixId' that is missing from readiness_status.json."
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
Write-Host "Status dates match across readiness_status.json and canonical readiness docs."
Write-Host "Release/template matrix rows match readiness_status.json in both coverage and status."
Write-Host "All READY subsystem evidence fields are true."
Write-Host "All READY/PARTIAL template required subsystems are known."
