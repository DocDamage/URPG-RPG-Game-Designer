$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$roadmapPath = Join-Path $repoRoot "docs\NATIVE_FEATURE_ABSORPTION_PLAN.md"
$statusPath = Join-Path $repoRoot "docs\PROGRAM_COMPLETION_STATUS.md"
$rulesPath = Join-Path $repoRoot "docs\TRUTH_ALIGNMENT_RULES.md"
$matrixPath = Join-Path $repoRoot "docs\RELEASE_READINESS_MATRIX.md"
$templateMatrixPath = Join-Path $repoRoot "docs\TEMPLATE_READINESS_MATRIX.md"

$requiredFiles = @($roadmapPath, $statusPath, $rulesPath, $matrixPath, $templateMatrixPath)
foreach ($file in $requiredFiles) {
    if (-not (Test-Path $file)) {
        throw "Missing truth-alignment input: $file"
    }
}

$roadmapText = Get-Content -Raw -Path $roadmapPath
$statusText = Get-Content -Raw -Path $statusPath

if ($roadmapText -match "\[x\]\s+Release-readiness matrix by subsystem\.") {
    throw "Roadmap still overclaims subsystem release-readiness matrix as landed."
}

if ($statusText -notmatch "subsystem-wide release-readiness matrix is still not landed") {
    throw "Program status must explicitly state that subsystem-wide release-readiness matrix is not yet landed."
}

if ($statusText -notmatch "template readiness matrix and template-claim guardrails are still not landed") {
    throw "Program status must explicitly state that template readiness governance is not yet landed."
}

Write-Host "Canonical truth-alignment checks passed."
