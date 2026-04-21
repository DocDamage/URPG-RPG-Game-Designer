$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$readinessPath = Join-Path $repoRoot "content/readiness/readiness_status.json"
$releaseMatrixPath = Join-Path $repoRoot "docs/RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs/TEMPLATE_READINESS_MATRIX.md"
$changelogPath = Join-Path $repoRoot "docs/SCHEMA_CHANGELOG.md"
$docsDir = Join-Path $repoRoot "docs"
$schemasDir = Join-Path $repoRoot "content/schemas"

foreach ($file in @($readinessPath, $releaseMatrixPath, $templateMatrixPath, $changelogPath)) {
    if (-not (Test-Path $file)) {
        throw "Missing required file: $file"
    }
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$releaseMatrixText = Get-Content -Raw -Path $releaseMatrixPath
$templateMatrixText = Get-Content -Raw -Path $templateMatrixPath
$changelogText = Get-Content -Raw -Path $changelogPath

$tick = [string][char]96
$mismatches = @()

# ---------------------------------------------------------------------------
# a. Every subsystem in readiness_status.json has a row in RELEASE_READINESS_MATRIX.md
# ---------------------------------------------------------------------------
foreach ($entry in $readiness.subsystems) {
    $pattern = [regex]::Escape($tick + $entry.id + $tick)
    if ($releaseMatrixText -notmatch $pattern) {
        $mismatches += "Subsystem '$($entry.id)' is missing from RELEASE_READINESS_MATRIX.md."
    }
}

# ---------------------------------------------------------------------------
# b. Every template in readiness_status.json has a row in TEMPLATE_READINESS_MATRIX.md
# ---------------------------------------------------------------------------
foreach ($entry in $readiness.templates) {
    $pattern = [regex]::Escape($tick + $entry.id + $tick)
    if ($templateMatrixText -notmatch $pattern) {
        $mismatches += "Template '$($entry.id)' is missing from TEMPLATE_READINESS_MATRIX.md."
    }
}

# ---------------------------------------------------------------------------
# c. Every READY subsystem has all evidence fields set to true
# ---------------------------------------------------------------------------
$evidenceFields = @("runtimeOwner", "editorSurface", "schemaMigration", "diagnostics", "testsValidation", "docsAligned")
foreach ($entry in $readiness.subsystems) {
    if ($entry.status -eq "READY") {
        foreach ($field in $evidenceFields) {
            if ($entry.evidence.$field -ne $true) {
                $mismatches += "Subsystem '$($entry.id)' is READY but evidence.$field is not true."
            }
        }
    }
}

# ---------------------------------------------------------------------------
# d. READY or PARTIAL templates only reference known subsystem IDs
# ---------------------------------------------------------------------------
$knownSubsystemIds = @{}
foreach ($entry in $readiness.subsystems) {
    $knownSubsystemIds[$entry.id] = $true
}

foreach ($entry in $readiness.templates) {
    if ($entry.status -eq "READY" -or $entry.status -eq "PARTIAL") {
        foreach ($requiredId in $entry.requiredSubsystems) {
            if (-not $knownSubsystemIds.ContainsKey($requiredId)) {
                $mismatches += "Template '$($entry.id)' references unknown subsystem '$requiredId'."
            }
        }
    }
}

# ---------------------------------------------------------------------------
# e. Overclaim check: docs must not claim non-READY subsystems are READY
# ---------------------------------------------------------------------------
$skipDocs = @("PROGRAM_COMPLETION_STATUS.md", "NATIVE_FEATURE_ABSORPTION_PLAN.md")
$docFiles = Get-ChildItem -Path $docsDir -Filter "*.md" -Recurse -File | Where-Object {
    $skipDocs -notcontains $_.Name
}

$readySubsystemIds = @{}
foreach ($entry in $readiness.subsystems) {
    if ($entry.status -eq "READY") {
        $readySubsystemIds[$entry.id] = $true
    }
}

foreach ($docFile in $docFiles) {
    $lines = Get-Content -Path $docFile.FullName
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        if ($line -notmatch 'is READY|are READY') {
            continue
        }

        foreach ($entry in $readiness.subsystems) {
            if ($entry.status -eq "READY") {
                continue
            }

            $id = $entry.id
            $backtickPattern = [regex]::Escape($tick + $id + $tick)
            $wordPattern = '\b' + [regex]::Escape($id) + '\b'

            if ($line -match $backtickPattern -or $line -match $wordPattern) {
                $mismatches += "Overclaim in '$($docFile.Name)' line $($i + 1): subsystem '$id' (status $($entry.status)) appears with 'is READY' or 'are READY'."
            }
        }
    }
}

# ---------------------------------------------------------------------------
# f. Every .schema.json under content/schemas is mentioned in SCHEMA_CHANGELOG.md
# ---------------------------------------------------------------------------
$schemaFiles = Get-ChildItem -Path $schemasDir -Filter "*.schema.json" -File
foreach ($schemaFile in $schemaFiles) {
    if ($changelogText -notmatch [regex]::Escape($schemaFile.Name)) {
        $mismatches += "Schema file '$($schemaFile.Name)' is not mentioned in SCHEMA_CHANGELOG.md."
    }
}

# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------
if ($mismatches.Count -gt 0) {
    foreach ($msg in $mismatches) {
        Write-Host "MISMATCH: $msg"
    }
    throw "Truth reconciliation failed with $($mismatches.Count) mismatch(es)."
}

Write-Host "All truth-reconciliation checks passed."
