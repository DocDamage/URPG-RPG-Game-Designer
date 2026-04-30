$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$readinessPath = Join-Path $repoRoot "content/readiness/readiness_status.json"
$releaseMatrixPath = Join-Path $repoRoot "docs/RELEASE_READINESS_MATRIX.md"
$appReleaseMatrixPath = Join-Path $repoRoot "docs/APP_RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs/TEMPLATE_READINESS_MATRIX.md"
$changelogPath = Join-Path $repoRoot "docs/SCHEMA_CHANGELOG.md"
$truthRulesPath = Join-Path $repoRoot "docs/TRUTH_ALIGNMENT_RULES.md"
$templateLabelRulesPath = Join-Path $repoRoot "docs/TEMPLATE_LABEL_RULES.md"
$subsystemStatusRulesPath = Join-Path $repoRoot "docs/SUBSYSTEM_STATUS_RULES.md"
$projectAuditDocPath = Join-Path $repoRoot "docs/PROJECT_AUDIT.md"
$compatSignoffPath = Join-Path $repoRoot "docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md"
$releaseSignoffWorkflowPath = Join-Path $repoRoot "docs/RELEASE_SIGNOFF_WORKFLOW.md"
$docsDir = Join-Path $repoRoot "docs"
$schemasDir = Join-Path $repoRoot "content/schemas"

foreach ($file in @(
        $readinessPath,
        $releaseMatrixPath,
        $appReleaseMatrixPath,
        $templateMatrixPath,
        $changelogPath,
        $truthRulesPath,
        $templateLabelRulesPath,
        $subsystemStatusRulesPath,
        $projectAuditDocPath,
        $compatSignoffPath,
        $releaseSignoffWorkflowPath
    )) {
    if (-not (Test-Path $file)) {
        throw "Missing required file: $file"
    }
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$releaseMatrixText = Get-Content -Raw -Path $releaseMatrixPath
$appReleaseMatrixText = Get-Content -Raw -Path $appReleaseMatrixPath
$templateMatrixText = Get-Content -Raw -Path $templateMatrixPath
$changelogText = Get-Content -Raw -Path $changelogPath
$truthRulesText = Get-Content -Raw -Path $truthRulesPath
$templateLabelRulesText = Get-Content -Raw -Path $templateLabelRulesPath
$subsystemStatusRulesText = Get-Content -Raw -Path $subsystemStatusRulesPath
$projectAuditDocText = Get-Content -Raw -Path $projectAuditDocPath
$compatSignoffText = Get-Content -Raw -Path $compatSignoffPath
$releaseSignoffWorkflowText = Get-Content -Raw -Path $releaseSignoffWorkflowPath
$battleSignoffPath = Join-Path $repoRoot "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"
$saveSignoffPath = Join-Path $repoRoot "docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"
$battleSignoffText = Get-Content -Raw -Path $battleSignoffPath
$saveSignoffText = Get-Content -Raw -Path $saveSignoffPath
$releaseSignoffWorkflowRelativePath = "docs/RELEASE_SIGNOFF_WORKFLOW.md"
$signoffArtifactMap = @{
    "battle_core" = "docs/BATTLE_CORE_CLOSURE_SIGNOFF.md"
    "save_data_core" = "docs/SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"
    "compat_bridge_exit" = "docs/COMPAT_BRIDGE_EXIT_SIGNOFF.md"
}

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

function Get-CurrentMatrixStatusRows {
    param(
        [string]$Text
    )

    $rows = @()
    $seenCurrentMatrix = $false
    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match '^## Current') {
            $seenCurrentMatrix = $true
            continue
        }
        if (-not $seenCurrentMatrix) {
            continue
        }
        if ($line -match '^## ') {
            break
        }
        if ($line -match '^\| (?:`([^`]+)`|([^|`]+?)) \| `([^`]+)` \|') {
            $id = if ($matches[1]) { $matches[1] } else { $matches[2].Trim() }
            $rows += @{ Id = $id; Status = $matches[3]; Line = $line }
        }
    }
    return $rows
}

function Get-FixtureOnlyOverclaimTerms {
    param(
        [string]$Text
    )

    $terms = @(
        "fixture-only",
        "fixture only",
        "mock-only",
        "mock only",
        "mock-backed",
        "mock backed",
        "placeholder-backed",
        "placeholder backed",
        "dev bootstrap"
    )

    $hits = @()
    $lower = $Text.ToLowerInvariant()
    foreach ($term in $terms) {
        if ($lower.Contains($term)) {
            $hits += $term
        }
    }
    return $hits
}

$docDates = @(
    @{ Label = "RELEASE_READINESS_MATRIX.md"; Value = (Get-StatusDateFromText -Text $releaseMatrixText -Label "RELEASE_READINESS_MATRIX.md") },
    @{ Label = "TEMPLATE_READINESS_MATRIX.md"; Value = (Get-StatusDateFromText -Text $templateMatrixText -Label "TEMPLATE_READINESS_MATRIX.md") },
    @{ Label = "TRUTH_ALIGNMENT_RULES.md"; Value = (Get-StatusDateFromText -Text $truthRulesText -Label "TRUTH_ALIGNMENT_RULES.md") },
    @{ Label = "TEMPLATE_LABEL_RULES.md"; Value = (Get-StatusDateFromText -Text $templateLabelRulesText -Label "TEMPLATE_LABEL_RULES.md") },
    @{ Label = "SUBSYSTEM_STATUS_RULES.md"; Value = (Get-StatusDateFromText -Text $subsystemStatusRulesText -Label "SUBSYSTEM_STATUS_RULES.md") },
    @{ Label = "PROJECT_AUDIT.md"; Value = (Get-StatusDateFromText -Text $projectAuditDocText -Label "PROJECT_AUDIT.md") },
    @{ Label = "RELEASE_SIGNOFF_WORKFLOW.md"; Value = (Get-StatusDateFromText -Text $releaseSignoffWorkflowText -Label "RELEASE_SIGNOFF_WORKFLOW.md") }
)

foreach ($docDate in $docDates) {
    if ($docDate.Value -and $docDate.Value -ne $readiness.statusDate) {
        $mismatches += "$($docDate.Label) status date '$($docDate.Value)' does not match readiness_status.json date '$($readiness.statusDate)'."
    }
}

$releaseRows = Get-MatrixRows -Text $releaseMatrixText
$templateRows = Get-MatrixRows -Text $templateMatrixText

# ---------------------------------------------------------------------------
# a0. READY/VERIFIED claims must not be justified by fixture-only, mock-only,
#     placeholder-backed, or dev-bootstrap evidence.
# ---------------------------------------------------------------------------
foreach ($entry in $readiness.subsystems) {
    if ($entry.status -ne "READY") {
        continue
    }

    $claimText = @($entry.summary) + @($entry.mainGaps) -join " "
    $hits = Get-FixtureOnlyOverclaimTerms -Text $claimText
    foreach ($hit in $hits) {
        $mismatches += "READY subsystem '$($entry.id)' contains release-overclaim term '$hit' in readiness_status.json."
    }
}

foreach ($entry in $readiness.templates) {
    if ($entry.status -ne "READY") {
        continue
    }

    $claimText = @($entry.safeScope) + @($entry.mainBlockers) + @($entry.barEvidence.PSObject.Properties.Value) -join " "
    $hits = Get-FixtureOnlyOverclaimTerms -Text $claimText
    foreach ($hit in $hits) {
        $mismatches += "READY template '$($entry.id)' contains release-overclaim term '$hit' in readiness_status.json."
    }
}

foreach ($matrix in @(
        @{ Name = "RELEASE_READINESS_MATRIX.md"; Text = $releaseMatrixText; ReadyStatuses = @("READY") },
        @{ Name = "TEMPLATE_READINESS_MATRIX.md"; Text = $templateMatrixText; ReadyStatuses = @("READY") },
        @{ Name = "APP_RELEASE_READINESS_MATRIX.md"; Text = $appReleaseMatrixText; ReadyStatuses = @("VERIFIED") }
    )) {
    foreach ($row in Get-CurrentMatrixStatusRows -Text $matrix.Text) {
        if ($matrix.ReadyStatuses -notcontains $row.Status) {
            continue
        }

        $hits = Get-FixtureOnlyOverclaimTerms -Text $row.Line
        foreach ($hit in $hits) {
            $mismatches += "$($matrix.Name) row '$($row.Id)' is $($row.Status) but contains release-overclaim term '$hit'."
        }
    }
}

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
        "input/localization-bundle/export",
        "localization consistency report",
        "governance detail",
        "release-signoff workflow"
    )) {
    if ($projectAuditDocText -notmatch [regex]::Escape($requiredPhrase)) {
        $mismatches += "PROJECT_AUDIT.md is missing expected governance phrase '$requiredPhrase'."
    }
}

# ---------------------------------------------------------------------------
# h. Release signoff workflow doc must match the shipped workflow pattern
# ---------------------------------------------------------------------------
foreach ($requiredPhrase in @(
        "does **not** grant release approval",
        "human review",
        "check_release_readiness.ps1",
        "truth_reconciler.ps1",
        "signoff artifact"
    )) {
    if ($releaseSignoffWorkflowText -notmatch [regex]::Escape($requiredPhrase)) {
        $mismatches += "RELEASE_SIGNOFF_WORKFLOW.md is missing expected phrase '$requiredPhrase'."
    }
}

# ---------------------------------------------------------------------------
# i. Signoff artifacts must match their current review state
# ---------------------------------------------------------------------------
foreach ($signoffDoc in @(
        @{ Name = "BATTLE_CORE_CLOSURE_SIGNOFF.md"; Text = $battleSignoffText; SubsystemId = "battle_core"; ReadyPhrases = @("Status:** ``READY``", "approved by release-owner review"); PendingPhrases = @("Human review is required", "residual gaps", "PARTIAL") },
        @{ Name = "SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"; Text = $saveSignoffText; SubsystemId = "save_data_core"; ReadyPhrases = @("Status:** ``READY``", "approved by release-owner review"); PendingPhrases = @("Human review is required", "residual gaps", "PARTIAL") },
        @{ Name = "COMPAT_BRIDGE_EXIT_SIGNOFF.md"; Text = $compatSignoffText; SubsystemId = "compat_bridge_exit"; ReadyPhrases = @("Compat Bridge Exit", "Status:** ``READY``", "approved by release-owner review"); PendingPhrases = @("Compat Bridge Exit", "Human review is required", "compat bridge exit", "residual gaps", "PARTIAL") }
    )) {
    $entry = $readiness.subsystems | Where-Object { $_.id -eq $signoffDoc.SubsystemId } | Select-Object -First 1
    $requiredPhrases = if ($entry -and $entry.status -eq "READY") { $signoffDoc.ReadyPhrases } else { $signoffDoc.PendingPhrases }
    foreach ($requiredPhrase in $requiredPhrases) {
        if ($signoffDoc.Text -notmatch [regex]::Escape($requiredPhrase)) {
            $mismatches += "$($signoffDoc.Name) is missing expected phrase '$requiredPhrase'."
        }
    }
}

# ---------------------------------------------------------------------------
# j. Structured signoff contracts must exist for all signoff-governed subsystem lanes
# ---------------------------------------------------------------------------
foreach ($subsystemId in $signoffArtifactMap.Keys) {
    $entry = $readiness.subsystems | Where-Object { $_.id -eq $subsystemId } | Select-Object -First 1
    if (-not $entry) {
        $mismatches += "Signoff-governed subsystem '$subsystemId' is missing from readiness_status.json."
        continue
    }

    if (-not ($entry.PSObject.Properties.Name -contains "signoff")) {
        $mismatches += "Signoff-governed subsystem '$subsystemId' is missing structured signoff contract data in readiness_status.json."
        continue
    }

    if ($entry.signoff.required -ne $true) {
        $mismatches += "Subsystem '$subsystemId' must set signoff.required to true."
    }

    if ([string]$entry.signoff.artifactPath -ne $signoffArtifactMap[$subsystemId]) {
        $mismatches += "Subsystem '$subsystemId' signoff.artifactPath must be '$($signoffArtifactMap[$subsystemId])'."
    }

    if ($entry.status -eq "READY") {
        if ($entry.signoff.promotionRequiresHumanReview -ne $false) {
            $mismatches += "READY subsystem '$subsystemId' must set signoff.promotionRequiresHumanReview to false."
        }
        if ([string]$entry.signoff.reviewStatus -ne "APPROVED") {
            $mismatches += "READY subsystem '$subsystemId' must set signoff.reviewStatus to APPROVED."
        }
    } elseif ($entry.signoff.promotionRequiresHumanReview -ne $true) {
        $mismatches += "Subsystem '$subsystemId' must keep signoff.promotionRequiresHumanReview set to true while it is not READY."
    }

    if ([string]$entry.signoff.workflow -ne $releaseSignoffWorkflowRelativePath) {
        $mismatches += "Subsystem '$subsystemId' signoff.workflow must be '$releaseSignoffWorkflowRelativePath'."
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
