param(
  [string]$BuildDirectory = "build/dev-ninja-debug",
  [string]$ExpectedCommit = "",
  [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"

function Get-RequiredProperty {
  param(
    [Parameter(Mandatory = $true)]$Object,
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][string]$Context
  )

  $property = $Object.PSObject.Properties[$Name]
  $missing = $null -eq $property -or $null -eq $property.Value
  if (-not $missing -and $property.Value -is [array]) {
    $missing = $property.Value.Count -eq 0
  } elseif (-not $missing) {
    $missing = [string]::IsNullOrWhiteSpace([string]$property.Value)
  }
  if ($missing) {
    throw "$Context is missing required property '$Name'."
  }
  return $property.Value
}

function Get-StringArray {
  param(
    [Parameter(Mandatory = $true)]$Object,
    [Parameter(Mandatory = $true)][string]$Name,
    [Parameter(Mandatory = $true)][string]$Context
  )

  $property = $Object.PSObject.Properties[$Name]
  if ($null -eq $property -or $null -eq $property.Value) {
    throw "$Context is missing required array '$Name'."
  }
  return @($property.Value | ForEach-Object { [string]$_ })
}

function Assert-ArtifactHash {
  param(
    [Parameter(Mandatory = $true)][string]$Path,
    [Parameter(Mandatory = $true)][string]$ExpectedHash,
    [Parameter(Mandatory = $true)][string]$Context
  )

  if ($ExpectedHash -notmatch '^[0-9a-fA-F]{64}$') {
    throw "$Context must provide a SHA-256 hex digest."
  }
  $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()
  if ($actualHash -ne $ExpectedHash.ToLowerInvariant()) {
    throw "$Context hash does not match the referenced artifact."
  }
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$buildDirectory = if ([System.IO.Path]::IsPathRooted($BuildDirectory)) {
  [System.IO.Path]::GetFullPath($BuildDirectory)
} else {
  [System.IO.Path]::GetFullPath((Join-Path $repoRoot $BuildDirectory))
}
if ([string]::IsNullOrWhiteSpace($ReportPath)) {
  $ReportPath = Join-Path $buildDirectory "creator_journey_qualification_report.json"
} elseif (-not [System.IO.Path]::IsPathRooted($ReportPath)) {
  $ReportPath = Join-Path $repoRoot $ReportPath
}

$specPath = Join-Path $repoRoot "content\fixtures\creator_journey_qualification_spec.json"
foreach ($path in @($specPath, $ReportPath)) {
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
    throw "Creator-journey qualification input is missing: $path"
  }
}

$spec = Get-Content -LiteralPath $specPath -Raw | ConvertFrom-Json
$report = Get-Content -LiteralPath $ReportPath -Raw | ConvertFrom-Json
if ($spec.schema -ne "urpg.creator_journey_qualification_spec.v1") {
  throw "Unsupported creator-journey qualification spec schema: $($spec.schema)"
}
if ($report.schema -ne "urpg.creator_journey_qualification_report.v1") {
  throw "Unsupported creator-journey qualification report schema: $($report.schema)"
}
if ($report.qualification_id -ne $spec.qualification_id) {
  throw "Qualification report targets '$($report.qualification_id)', expected '$($spec.qualification_id)'."
}
if ($report.status -ne "passed") {
  throw "Creator-journey qualification report status must be 'passed', found '$($report.status)'."
}

if ([string]::IsNullOrWhiteSpace($ExpectedCommit)) {
  $ExpectedCommit = (& git -C $repoRoot rev-parse HEAD).Trim()
  if ($LASTEXITCODE -ne 0) { throw "Unable to resolve the current Git commit." }
}
[void](Get-RequiredProperty -Object $report -Name "provenance" -Context "Qualification report")
$provenance = $report.provenance
if ((Get-RequiredProperty -Object $provenance -Name "source_commit" -Context "Qualification provenance") -ne $ExpectedCommit) {
  throw "Qualification provenance source_commit does not match expected commit '$ExpectedCommit'."
}
if ((Get-RequiredProperty -Object $provenance -Name "clean_worktree" -Context "Qualification provenance") -ne $true) {
  throw "Qualification provenance does not prove a clean worktree."
}
if ($null -eq $provenance.PSObject.Properties['builds'] -or $null -eq $provenance.builds) {
  throw "Qualification provenance is missing required property 'builds'."
}
$builds = @($provenance.builds)
foreach ($configuration in @($spec.required_provenance.build_configurations)) {
  $build = @($builds | Where-Object { $_.configuration -eq $configuration })
  if ($build.Count -ne 1) {
    throw "Qualification provenance must contain exactly one '$configuration' build."
  }
  foreach ($field in @("preset", "platform", "compiler", "binary_sha256")) {
    [void](Get-RequiredProperty -Object $build[0] -Name $field -Context "Qualification $configuration build")
  }
  if ($build[0].source_commit -ne $ExpectedCommit) {
    throw "Qualification $configuration build provenance does not match expected commit '$ExpectedCommit'."
  }
}

$expectedSteps = @($spec.steps)
$actualSteps = @($report.steps)
if ($actualSteps.Count -ne $expectedSteps.Count) {
  throw "Qualification report has $($actualSteps.Count) steps; expected $($expectedSteps.Count)."
}
$actualById = @{}
foreach ($step in $actualSteps) {
  $id = Get-RequiredProperty -Object $step -Name "id" -Context "Qualification step"
  if ($actualById.ContainsKey($id)) { throw "Qualification report contains duplicate step '$id'." }
  $actualById[$id] = $step
}

foreach ($specStep in $expectedSteps) {
  $id = $specStep.id
  if (-not $actualById.ContainsKey($id)) { throw "Qualification report is missing required step '$id'." }
  $step = $actualById[$id]
  if ($step.status -ne "passed") { throw "Qualification step '$id' must be passed, found '$($step.status)'." }
  if ([int64]$step.duration_ms -gt [int64]$specStep.budget.max_duration_ms) {
    throw "Qualification step '$id' exceeded its deterministic duration budget."
  }
  $diagnostics = Get-StringArray -Object $step -Name "diagnostic_codes" -Context "Qualification step '$id'"
  foreach ($diagnostic in $diagnostics) {
    if ($diagnostic -in @($specStep.forbidden_diagnostic_codes)) {
      throw "Qualification step '$id' contains forbidden diagnostic '$diagnostic'."
    }
    if ($diagnostic -notin @($specStep.allowed_informational_diagnostics)) {
      throw "Qualification step '$id' contains unsupported diagnostic '$diagnostic'."
    }
  }
  [void](Get-RequiredProperty -Object $step -Name "evidence" -Context "Qualification step '$id'")
  $evidence = @($step.evidence)
  $evidenceKinds = @()
  foreach ($item in @($evidence)) {
    $kind = Get-RequiredProperty -Object $item -Name "kind" -Context "Qualification evidence for '$id'"
    $artifact = Get-RequiredProperty -Object $item -Name "artifact_path" -Context "Qualification evidence for '$id'"
    $expectedHash = Get-RequiredProperty -Object $item -Name "sha256" -Context "Qualification evidence for '$id'"
    if ((Get-RequiredProperty -Object $item -Name "source_commit" -Context "Qualification evidence for '$id'") -ne $ExpectedCommit) {
      throw "Qualification evidence '$kind' for '$id' does not match expected commit '$ExpectedCommit'."
    }
    if (-not (Test-Path -LiteralPath $artifact -PathType Leaf)) {
      throw "Qualification evidence '$kind' for '$id' points to a missing artifact: $artifact"
    }
    Assert-ArtifactHash -Path $artifact -ExpectedHash $expectedHash -Context "Qualification evidence '$kind' for '$id'"
    $evidenceKinds += $kind
  }
  foreach ($requiredKind in @($specStep.required_evidence_kinds)) {
    if ($requiredKind -notin $evidenceKinds) {
      throw "Qualification step '$id' is missing required evidence kind '$requiredKind'."
    }
  }
}

foreach ($actualId in $actualById.Keys) {
  if ($actualId -notin @($expectedSteps | ForEach-Object { $_.id })) {
    throw "Qualification report contains unknown step '$actualId'."
  }
}

Write-Host "Creator-journey PFU-I1 qualification passed: $ReportPath"
