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

if ($roadmapText -notmatch "\[x\]\s+Release-readiness matrix by subsystem\.") {
    throw "Roadmap must mark release-readiness matrix as landed."
}

if ($statusText -notmatch "subsystem-wide release-readiness matrix" -and $statusText -notmatch "RELEASE_READINESS_MATRIX") {
    throw "Program status must reference the subsystem-wide release-readiness matrix."
}

if ($statusText -notmatch "template readiness matrix" -and $statusText -notmatch "TEMPLATE_READINESS_MATRIX") {
    throw "Program status must reference the template readiness matrix."
}

Write-Host "Canonical truth-alignment checks passed."
