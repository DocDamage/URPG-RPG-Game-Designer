$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$readinessPath = Join-Path $repoRoot "content/readiness/readiness_status.json"
$releaseMatrixPath = Join-Path $repoRoot "docs/RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs/TEMPLATE_READINESS_MATRIX.md"
$changelogPath = Join-Path $repoRoot "docs/SCHEMA_CHANGELOG.md"
$truthRulesPath = Join-Path $repoRoot "docs/TRUTH_ALIGNMENT_RULES.md"
$templateLabelRulesPath = Join-Path $repoRoot "docs/TEMPLATE_LABEL_RULES.md"
$subsystemStatusRulesPath = Join-Path $repoRoot "docs/SUBSYSTEM_STATUS_RULES.md"
$projectAuditDocPath = Join-Path $repoRoot "docs/PROJECT_AUDIT.md"
$compatSignoffPath = Join-Path $repoRoot "docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md"
$docsDir = Join-Path $repoRoot "docs"
$schemasDir = Join-Path $repoRoot "content/schemas"

foreach ($file in @(
        $readinessPath,
        $releaseMatrixPath,
        $templateMatrixPath,
        $changelogPath,
        $truthRulesPath,
        $templateLabelRulesPath,
        $subsystemStatusRulesPath,
        $projectAuditDocPath,
        $compatSignoffPath
    )) {
    if (-not (Test-Path $file)) {
        throw "Missing required file: $file"
    }
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$releaseMatrixText = Get-Content -Raw -Path $releaseMatrixPath
$templateMatrixText = Get-Content -Raw -Path $templateMatrixPath
$changelogText = Get-Content -Raw -Path $changelogPath
$truthRulesText = Get-Content -Raw -Path $truthRulesPath
$templateLabelRulesText = Get-Content -Raw -Path $templateLabelRulesPath
$subsystemStatusRulesText = Get-Content -Raw -Path $subsystemStatusRulesPath
$projectAuditDocText = Get-Content -Raw -Path $projectAuditDocPath
$compatSignoffText = Get-Content -Raw -Path $compatSignoffPath
$battleSignoffPath = Join-Path $repoRoot "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"
$saveSignoffPath = Join-Path $repoRoot "docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"
$battleSignoffText = Get-Content -Raw -Path $battleSignoffPath
$saveSignoffText = Get-Content -Raw -Path $saveSignoffPath

$tick = [string][char]96
$mismatches = @()

function Get-StatusDateFromText {
    param(
        [string]$Text,
        [string]$Label
    )

    $match = [regex]::Match($Text, 'Status Date:\s*(\d{4}-\d{2}-\d{2})')
    if (-not $match.Success) {
        $script:mismatches += "$Label is missing a Status Date."
        return $null
    }
    return $match.Groups[1].Value
}

function Get-MatrixRows {
    param(
        [string]$Text
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
    return $rows
}

$docDates = @(
    @{ Label = "RELEASE_READINESS_MATRIX.md"; Value = (Get-StatusDateFromText -Text $releaseMatrixText -Label "RELEASE_READINESS_MATRIX.md") },
    @{ Label = "TEMPLATE_READINESS_MATRIX.md"; Value = (Get-StatusDateFromText -Text $templateMatrixText -Label "TEMPLATE_READINESS_MATRIX.md") },
    @{ Label = "TRUTH_ALIGNMENT_RULES.md"; Value = (Get-StatusDateFromText -Text $truthRulesText -Label "TRUTH_ALIGNMENT_RULES.md") },
    @{ Label = "TEMPLATE_LABEL_RULES.md"; Value = (Get-StatusDateFromText -Text $templateLabelRulesText -Label "TEMPLATE_LABEL_RULES.md") },
    @{ Label = "SUBSYSTEM_STATUS_RULES.md"; Value = (Get-StatusDateFromText -Text $subsystemStatusRulesText -Label "SUBSYSTEM_STATUS_RULES.md") },
    @{ Label = "PROJECT_AUDIT.md"; Value = (Get-StatusDateFromText -Text $projectAuditDocText -Label "PROJECT_AUDIT.md") }
)

foreach ($docDate in $docDates) {
    if ($docDate.Value -and $docDate.Value -ne $readiness.statusDate) {
        $mismatches += "$($docDate.Label) status date '$($docDate.Value)' does not match readiness_status.json date '$($readiness.statusDate)'."
    }
}

$releaseRows = Get-MatrixRows -Text $releaseMatrixText
$templateRows = Get-MatrixRows -Text $templateMatrixText

# ---------------------------------------------------------------------------
# a. Every subsystem in readiness_status.json has a row in RELEASE_READINESS_MATRIX.md
# ---------------------------------------------------------------------------
foreach ($entry in $readiness.subsystems) {
    if (-not $releaseRows.ContainsKey($entry.id)) {
        $mismatches += "Subsystem '$($entry.id)' is missing from RELEASE_READINESS_MATRIX.md."
        continue
    }
    if ($releaseRows[$entry.id] -ne $entry.status) {
        $mismatches += "Subsystem '$($entry.id)' has status '$($entry.status)' in readiness_status.json but '$($releaseRows[$entry.id])' in RELEASE_READINESS_MATRIX.md."
    }
}

# ---------------------------------------------------------------------------
# b. Every template in readiness_status.json has a row in TEMPLATE_READINESS_MATRIX.md
# ---------------------------------------------------------------------------
foreach ($entry in $readiness.templates) {
    if (-not $templateRows.ContainsKey($entry.id)) {
        $mismatches += "Template '$($entry.id)' is missing from TEMPLATE_READINESS_MATRIX.md."
        continue
    }
    if ($templateRows[$entry.id] -ne $entry.status) {
        $mismatches += "Template '$($entry.id)' has status '$($entry.status)' in readiness_status.json but '$($templateRows[$entry.id])' in TEMPLATE_READINESS_MATRIX.md."
    }
}

# ---------------------------------------------------------------------------
# b2. Matrices must not contain rows absent from readiness_status.json
# ---------------------------------------------------------------------------
foreach ($matrixId in $releaseRows.Keys) {
    if (-not ($readiness.subsystems | Where-Object { $_.id -eq $matrixId })) {
        $mismatches += "RELEASE_READINESS_MATRIX.md contains extra subsystem row '$matrixId' not present in readiness_status.json."
    }
}

foreach ($matrixId in $templateRows.Keys) {
    if (-not ($readiness.templates | Where-Object { $_.id -eq $matrixId })) {
        $mismatches += "TEMPLATE_READINESS_MATRIX.md contains extra template row '$matrixId' not present in readiness_status.json."
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
# g. PROJECT_AUDIT.md must describe the shipped richer governance sections
# ---------------------------------------------------------------------------
foreach ($requiredPhrase in @(
        "accessibility",
        "audio",
        "performance",
        "input/localization/export",
        "governance detail"
    )) {
    if ($projectAuditDocText -notmatch [regex]::Escape($requiredPhrase)) {
        $mismatches += "PROJECT_AUDIT.md is missing expected governance phrase '$requiredPhrase'."
    }
}

# ---------------------------------------------------------------------------
# h. Signoff artifacts must match the shipped human-review-gated pattern
# ---------------------------------------------------------------------------
foreach ($signoffDoc in @(
        @{ Name = "BATTLE_CORE_CLOSURE_SIGNOFF.md"; Text = $battleSignoffText; RequiredPhrases = @("Human review is required", "residual gaps", "PARTIAL") },
        @{ Name = "SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"; Text = $saveSignoffText; RequiredPhrases = @("Human review is required", "residual gaps", "PARTIAL") },
        @{ Name = "COMPAT_BRIDGE_EXIT_SIGNOFF.md"; Text = $compatSignoffText; RequiredPhrases = @("Compat Bridge Exit", "Human review is required", "compat bridge exit", "residual gaps", "PARTIAL") }
    )) {
    foreach ($requiredPhrase in $signoffDoc.RequiredPhrases) {
        if ($signoffDoc.Text -notmatch [regex]::Escape($requiredPhrase)) {
            $mismatches += "$($signoffDoc.Name) is missing expected phrase '$requiredPhrase'."
        }
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
