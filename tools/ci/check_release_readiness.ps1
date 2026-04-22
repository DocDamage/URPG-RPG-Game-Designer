$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$readinessPath = Join-Path $repoRoot "content\readiness\readiness_status.json"
$matrixPath = Join-Path $repoRoot "docs\RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs\TEMPLATE_READINESS_MATRIX.md"
$truthRulesPath = Join-Path $repoRoot "docs\TRUTH_ALIGNMENT_RULES.md"
$projectAuditDocPath = Join-Path $repoRoot "docs\PROJECT_AUDIT.md"
$releaseSignoffWorkflowPath = Join-Path $repoRoot "docs\RELEASE_SIGNOFF_WORKFLOW.md"
$signoffDocPaths = @{
    "battle_core" = Join-Path $repoRoot "docs\BATTLE_CORE_CLOSURE_SIGNOFF.md"
    "save_data_core" = Join-Path $repoRoot "docs\SAVE_DATA_CORE_CLOSURE_SIGNOFF.md"
    "compat_bridge_exit" = Join-Path $repoRoot "docs\COMPAT_BRIDGE_EXIT_SIGNOFF.md"
}
$releaseSignoffWorkflowRelativePath = "docs/RELEASE_SIGNOFF_WORKFLOW.md"

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
if (-not (Test-Path $projectAuditDocPath)) {
    throw "Missing project audit doc: $projectAuditDocPath"
}
if (-not (Test-Path $releaseSignoffWorkflowPath)) {
    throw "Missing release signoff workflow doc: $releaseSignoffWorkflowPath"
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

function Get-MatrixRowText {
    param(
        [string]$Text,
        [string]$Id
    )

    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match ('^\| `' + [regex]::Escape($Id) + '` \|')) {
            return $line
        }
    }

    return $null
}

function Get-ProjectAuditExecutable {
    param(
        [string]$RepoRoot
    )

    $buildRoot = Join-Path $RepoRoot "build"
    if (-not (Test-Path $buildRoot)) {
        throw "Missing build directory: $buildRoot. Build the repo before running check_release_readiness.ps1."
    }

    $matches = Get-ChildItem -Path $buildRoot -Recurse -File | Where-Object {
        $_.BaseName -eq "urpg_project_audit" -and ($_.Extension -eq ".exe" -or [string]::IsNullOrEmpty($_.Extension))
    } | Sort-Object -Property @(
        @{ Expression = "LastWriteTime"; Descending = $true },
        @{ Expression = "FullName"; Descending = $false }
    )

    if ($matches) {
        return ($matches | Select-Object -First 1).FullName
    }

    throw "Could not find urpg_project_audit executable under $buildRoot. Build the repo before running check_release_readiness.ps1."
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
$projectAuditDocText = Get-Content -Raw -Path $projectAuditDocPath
$releaseSignoffWorkflowText = Get-Content -Raw -Path $releaseSignoffWorkflowPath

$releaseMatrixDate = Get-StatusDateFromText -Text $matrixText -Label "Release readiness matrix"
$templateMatrixDate = Get-StatusDateFromText -Text $templateMatrixText -Label "Template readiness matrix"
$truthRulesDate = Get-StatusDateFromText -Text $truthRulesText -Label "Truth alignment rules"
$projectAuditDocDate = Get-StatusDateFromText -Text $projectAuditDocText -Label "Project audit doc"
$releaseSignoffWorkflowDate = Get-StatusDateFromText -Text $releaseSignoffWorkflowText -Label "Release signoff workflow doc"

foreach ($docDate in @(
        @{ Label = "Release readiness matrix"; Value = $releaseMatrixDate },
        @{ Label = "Template readiness matrix"; Value = $templateMatrixDate },
        @{ Label = "Truth alignment rules"; Value = $truthRulesDate },
        @{ Label = "Project audit doc"; Value = $projectAuditDocDate },
        @{ Label = "Release signoff workflow doc"; Value = $releaseSignoffWorkflowDate }
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

foreach ($subsystemId in $signoffDocPaths.Keys) {
    if (-not (Test-Path $signoffDocPaths[$subsystemId])) {
        throw "Missing required signoff artifact for subsystem '$subsystemId': $($signoffDocPaths[$subsystemId])"
    }

    $readinessEntry = $readiness.subsystems | Where-Object { $_.id -eq $subsystemId } | Select-Object -First 1
    if (-not $readinessEntry) {
        throw "Signoff-governed subsystem '$subsystemId' is missing from readiness_status.json."
    }

    $summaryText = [string]$readinessEntry.summary
    $mainGapText = (($readinessEntry.mainGaps | ForEach-Object { [string]$_ }) -join " ")
    if ($summaryText -notmatch "signoff|human review") {
        throw "Signoff-governed subsystem '$subsystemId' must mention signoff or human review in readiness_status.json summary."
    }
    if ($mainGapText -notmatch "signoff|human review") {
        throw "Signoff-governed subsystem '$subsystemId' must mention signoff or human review in readiness_status.json mainGaps."
    }

    $rowText = Get-MatrixRowText -Text $matrixText -Id $subsystemId
    if (-not $rowText) {
        throw "Release readiness matrix row text for signoff-governed subsystem '$subsystemId' could not be found."
    }
    if ($rowText -notmatch "signoff|human review") {
        throw "Release readiness matrix row for signoff-governed subsystem '$subsystemId' must mention signoff or human review."
    }

    if (-not ($readinessEntry.PSObject.Properties.Name -contains "signoff")) {
        throw "Signoff-governed subsystem '$subsystemId' must declare a structured signoff contract in readiness_status.json."
    }

    $signoff = $readinessEntry.signoff
    if ($signoff.required -ne $true) {
        throw "Signoff-governed subsystem '$subsystemId' must set signoff.required to true."
    }

    $expectedRelativePath = $signoffDocPaths[$subsystemId].Substring($repoRoot.Path.Length).TrimStart('\', '/').Replace('\', '/')
    $actualArtifactPath = [string]$signoff.artifactPath
    if ($actualArtifactPath -ne $expectedRelativePath) {
        throw "Subsystem '$subsystemId' signoff.artifactPath must be '$expectedRelativePath' but was '$actualArtifactPath'."
    }

    if ($signoff.promotionRequiresHumanReview -ne $true) {
        throw "Subsystem '$subsystemId' must keep signoff.promotionRequiresHumanReview set to true."
    }

    $actualWorkflowPath = [string]$signoff.workflow
    if ($actualWorkflowPath -ne $releaseSignoffWorkflowRelativePath) {
        throw "Subsystem '$subsystemId' signoff.workflow must be '$releaseSignoffWorkflowRelativePath' but was '$actualWorkflowPath'."
    }

    if ($readinessEntry.status -eq "READY") {
        throw "Signoff-governed subsystem '$subsystemId' cannot be marked READY while the machine-checked signoff contract still requires human review."
    }
}

foreach ($requiredPhrase in @(
        "canonical workflow",
        "does not grant release approval",
        "human review",
        "check_release_readiness.ps1",
        "truth_reconciler.ps1"
    )) {
    if ($releaseSignoffWorkflowText -notmatch [regex]::Escape($requiredPhrase)) {
        throw "RELEASE_SIGNOFF_WORKFLOW.md is missing expected phrase '$requiredPhrase'."
    }
}

$projectAuditExecutable = Get-ProjectAuditExecutable -RepoRoot $repoRoot
$projectAuditJson = & $projectAuditExecutable --json --input $readinessPath 2>$null | Out-String
if (-not $projectAuditJson.Trim()) {
    throw "urpg_project_audit did not emit JSON output."
}

$projectAuditReport = $projectAuditJson | ConvertFrom-Json
foreach ($field in @("schemaVersion", "statusDate", "headline", "summary", "releaseBlockerCount", "exportBlockerCount", "templateContext", "governance", "issues")) {
    if (-not ($projectAuditReport.PSObject.Properties.Name -contains $field)) {
        throw "urpg_project_audit JSON is missing required top-level field '$field'."
    }
}

if ($projectAuditReport.statusDate -ne $readiness.statusDate) {
    throw "urpg_project_audit statusDate '$($projectAuditReport.statusDate)' does not match readiness_status.json date '$($readiness.statusDate)'."
}

foreach ($section in @(
        "assetReport",
        "schema",
        "projectSchema",
        "localizationArtifacts",
        "localizationEvidence",
        "inputArtifacts",
        "exportArtifacts",
        "accessibilityArtifacts",
        "audioArtifacts",
        "achievementArtifacts",
        "performanceArtifacts",
        "releaseSignoffWorkflow",
        "signoffArtifacts",
        "templateSpecArtifacts"
    )) {
    if (-not ($projectAuditReport.governance.PSObject.Properties.Name -contains $section)) {
        throw "urpg_project_audit governance is missing section '$section'."
    }
}

foreach ($countField in @(
        "assetGovernanceIssueCount",
        "schemaGovernanceIssueCount",
        "projectArtifactIssueCount",
        "localizationEvidenceIssueCount",
        "inputArtifactIssueCount",
        "accessibilityArtifactIssueCount",
        "audioArtifactIssueCount",
        "achievementArtifactIssueCount",
        "performanceArtifactIssueCount",
        "releaseSignoffWorkflowIssueCount",
        "signoffArtifactIssueCount",
        "templateSpecArtifactIssueCount"
    )) {
    if (-not ($projectAuditReport.PSObject.Properties.Name -contains $countField)) {
        throw "urpg_project_audit JSON is missing count field '$countField'."
    }
}

Write-Host "Release readiness records and matrices are present and minimally aligned."
Write-Host "Status dates match across readiness_status.json and canonical readiness docs."
Write-Host "Release/template matrix rows match readiness_status.json in both coverage and status."
Write-Host "All READY subsystem evidence fields are true."
Write-Host "All READY/PARTIAL template required subsystems are known."
Write-Host "Required signoff artifacts exist for human-review-gated subsystems."
Write-Host "Release signoff workflow artifact is present and carries the expected non-promoting workflow language."
Write-Host "urpg_project_audit JSON contract includes the required governance sections and issue counts."
