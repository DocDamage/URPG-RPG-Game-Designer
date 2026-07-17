param(
  [ValidateSet("A", "B", "C", "D", "E", "F", "G")]
  [string]$Wave = "A"
)

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$matrixPath = Join-Path $repoRoot ("content\readiness\product_completion_wave_{0}_gap_matrix.json" -f $Wave.ToLowerInvariant())
if (-not (Test-Path -LiteralPath $matrixPath -PathType Leaf)) {
  throw "Product-completion Wave $Wave gap matrix is missing: $matrixPath"
}

$matrix = Get-Content -LiteralPath $matrixPath -Raw | ConvertFrom-Json
if ($matrix.schema -ne "urpg.product_completion_wave_gap_matrix.v1" -or $matrix.wave -ne $Wave) {
  throw "Unsupported product-completion wave gap matrix schema or wave."
}

$expected = if ($Wave -eq "A") {
  @("PCQ-000", "PCQ-001", "PCQ-002", "PCQ-003", "PCQ-004", "PCQ-005", "PCQ-006") +
    @(100..108 | ForEach-Object { "PCQ-$_" }) +
    @(200..209 | ForEach-Object { "PCQ-$_" })
} elseif ($Wave -eq "B") {
  @(300..307 | ForEach-Object { "PCQ-$_" }) +
    @(350..357 | ForEach-Object { "PCQ-$_" })
} elseif ($Wave -eq "C") {
  @(400..407 | ForEach-Object { "PCQ-$_" }) +
    @(450..455 | ForEach-Object { "PCQ-$_" }) +
    @(480..485 | ForEach-Object { "PCQ-$_" }) +
    @("PCQ-654")
} elseif ($Wave -eq "D") {
  @(500..507 | ForEach-Object { "PCQ-$_" }) +
    @(600..607 | ForEach-Object { "PCQ-$_" }) +
    @(650..653 | ForEach-Object { "PCQ-$_" }) +
    @("PCQ-655", "PCQ-656")
} elseif ($Wave -eq "E") {
  @(700..707 | ForEach-Object { "PCQ-$_" }) +
    @(750..756 | ForEach-Object { "PCQ-$_" })
} elseif ($Wave -eq "F") {
  @(0..9 | ForEach-Object { "ARDY-{0:D3}" -f $_ }) +
    @(900..903 | ForEach-Object { "PCQ-$_" })
} else {
  @(904..906 | ForEach-Object { "PCQ-$_" })
}
$entries = @($matrix.entries)
$ids = @($entries | ForEach-Object { $_.requirementId })
$errors = [System.Collections.Generic.List[string]]::new()

foreach ($duplicate in @($ids | Group-Object | Where-Object Count -gt 1)) {
  $errors.Add("Duplicate Wave $Wave requirement '$($duplicate.Name)'.")
}
foreach ($id in $expected) {
  if ($id -notin $ids) { $errors.Add("Missing Wave $Wave requirement '$id'.") }
}
foreach ($id in $ids) {
  if ($id -notin $expected) { $errors.Add("Unexpected Wave $Wave requirement '$id'.") }
}

foreach ($entry in $entries) {
  if ($entry.classification -notin @("proved", "partial", "missing", "external")) {
    $errors.Add("$($entry.requirementId) has invalid classification '$($entry.classification)'.")
  }
  if (@($entry.existingProof).Count -eq 0) {
    $errors.Add("$($entry.requirementId) does not name existing proof.")
  }
  if ($entry.classification -ne "proved" -and @($entry.missingProof).Count -eq 0) {
    $errors.Add("$($entry.requirementId) does not name its exact missing proof.")
  }
  if ($entry.classification -eq "external" -and @($entry.externalDependencies).Count -eq 0) {
    $errors.Add("$($entry.requirementId) is external but names no external dependency.")
  }
}

$counts = @{}
foreach ($classification in @("proved", "partial", "missing", "external")) {
  $counts[$classification] = @($entries | Where-Object classification -eq $classification).Count
  if ($matrix.summary.$classification -ne $counts[$classification]) {
    $errors.Add("Summary count for '$classification' is stale.")
  }
}
if ($matrix.summary.required -ne $expected.Count -or $entries.Count -ne $expected.Count) {
  $errors.Add("Wave $Wave must contain exactly $($expected.Count) requirements.")
}

if ($errors.Count -gt 0) { throw ($errors -join [Environment]::NewLine) }
Write-Host "Product-completion Wave $Wave gap matrix passed: required=$($expected.Count), proved=$($counts.proved), partial=$($counts.partial), missing=$($counts.missing), external=$($counts.external)."
