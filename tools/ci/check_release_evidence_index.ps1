param(
  [switch]$AllowIncomplete,
  [int]$MaxEvidenceAgeDays = 30,
  [string]$ReportPath = "build/reports/product_completion/release_evidence_health.json"
)

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$indexPath = Join-Path $repoRoot "content\readiness\release_evidence_index.json"
if (-not (Test-Path -LiteralPath $indexPath -PathType Leaf)) {
  throw "Release evidence index is missing: $indexPath"
}
$index = Get-Content -LiteralPath $indexPath -Raw | ConvertFrom-Json
if ($index.schema -ne "urpg.release_evidence_index.v1") {
  throw "Unsupported release evidence index schema: $($index.schema)"
}
$authorityPath = Join-Path $repoRoot $index.authority
if (-not (Test-Path -LiteralPath $authorityPath -PathType Leaf)) {
  throw "Release evidence authority is missing: $authorityPath"
}

$planText = Get-Content -LiteralPath $authorityPath -Raw
$requiredIds = @([regex]::Matches($planText, '\| (PCQ-\d{3}|ARDY-\d{3}) \|') |
  ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
$head = (& git -C $repoRoot rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($head)) { throw "Unable to resolve HEAD." }

$byId = @{}
$errors = [System.Collections.Generic.List[string]]::new()
foreach ($entry in @($index.entries)) {
  foreach ($field in @("requirementId", "status", "command", "reports", "screenshots", "recordings", "hardware",
                       "sourceCommit", "dirtyState", "evidenceDate", "notes")) {
    if ($null -eq $entry.PSObject.Properties[$field]) {
      $errors.Add("Evidence entry is missing '$field'.")
    }
  }
  if ($byId.ContainsKey($entry.requirementId)) {
    $errors.Add("Duplicate evidence entry '$($entry.requirementId)'.")
  } else {
    $byId[$entry.requirementId] = $entry
  }
  if ($entry.requirementId -notin $requiredIds) {
    $errors.Add("Evidence index contains unknown requirement '$($entry.requirementId)'.")
  }
  if ($entry.status -notin @("passed", "blocked", "deferred", "in_progress")) {
    $errors.Add("Evidence entry '$($entry.requirementId)' has unsupported status '$($entry.status)'.")
  }
}

$missing = @($requiredIds | Where-Object { -not $byId.ContainsKey($_) })
$stale = [System.Collections.Generic.List[string]]::new()
foreach ($id in $requiredIds) {
  if (-not $byId.ContainsKey($id)) { continue }
  $entry = $byId[$id]
  if ($entry.status -ne "passed") { continue }
  if ([string]::IsNullOrWhiteSpace($entry.command) -or [string]::IsNullOrWhiteSpace($entry.hardware)) {
    $errors.Add("Passed evidence '$id' is missing command or hardware identity.")
  }
  if ($entry.sourceCommit -ne $head -or $entry.dirtyState -ne "clean") {
    $stale.Add("${id}: commit/dirty provenance does not match the clean current candidate.")
  }
  try {
    $date = [DateTimeOffset]::Parse($entry.evidenceDate)
    if ([DateTimeOffset]::UtcNow - $date.ToUniversalTime() -gt [TimeSpan]::FromDays($MaxEvidenceAgeDays)) {
      $stale.Add("${id}: evidence is older than $MaxEvidenceAgeDays days.")
    }
  } catch {
    $errors.Add("Passed evidence '$id' has an invalid evidenceDate.")
  }
  foreach ($artifact in @($entry.reports) + @($entry.screenshots) + @($entry.recordings)) {
    $artifactPath = if ([System.IO.Path]::IsPathRooted($artifact)) { $artifact } else { Join-Path $repoRoot $artifact }
    if (-not (Test-Path -LiteralPath $artifactPath -PathType Leaf)) {
      $errors.Add("Passed evidence '$id' references a missing artifact: $artifact")
    }
  }
}

$incomplete = @($requiredIds | Where-Object { -not $byId.ContainsKey($_) -or $byId[$_].status -ne "passed" })
$resolvedReport = if ([System.IO.Path]::IsPathRooted($ReportPath)) { $ReportPath } else { Join-Path $repoRoot $ReportPath }
$reportDirectory = Split-Path -Parent $resolvedReport
New-Item -ItemType Directory -Force -Path $reportDirectory | Out-Null
$report = [ordered]@{
  schema = "urpg.release_evidence_health.v1"
  generatedUtc = [DateTimeOffset]::UtcNow.ToString("o")
  sourceCommit = $head
  authority = $authorityPath
  requiredCount = $requiredIds.Count
  indexedCount = @($index.entries).Count
  passedCount = @($index.entries | Where-Object { $_.status -eq "passed" }).Count
  incompleteRequirementIds = $incomplete
  missingRequirementIds = $missing
  staleEvidence = @($stale)
  errors = @($errors)
  passed = $errors.Count -eq 0 -and $stale.Count -eq 0 -and $incomplete.Count -eq 0
}
[System.IO.File]::WriteAllText($resolvedReport, ($report | ConvertTo-Json -Depth 8) + [Environment]::NewLine)
Write-Host "Release evidence health: $($report.passed); required=$($report.requiredCount), indexed=$($report.indexedCount), passed=$($report.passedCount)."
Write-Host "Release evidence health report: $resolvedReport"
if ($errors.Count -gt 0) { throw ($errors -join [Environment]::NewLine) }
if (-not $AllowIncomplete -and ($stale.Count -gt 0 -or $incomplete.Count -gt 0)) {
  throw "Release evidence index is incomplete or stale: $($incomplete.Count) incomplete requirement(s), $($stale.Count) stale evidence row(s)."
}
if ($AllowIncomplete -and ($stale.Count -gt 0 -or $incomplete.Count -gt 0)) {
  Write-Warning "Incomplete implementation evidence is allowed for this check; release qualification will fail."
}
