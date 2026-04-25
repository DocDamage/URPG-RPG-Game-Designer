param(
  [string]$FixtureRoot = "",
  [switch]$ExpectFailure
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
if (-not $FixtureRoot) {
  $FixtureRoot = Join-Path $repoRoot "content\fixtures\feature_governance"
}

$errors = @()
$manifestPath = Join-Path $FixtureRoot "governance_manifest.json"

if (-not (Test-Path $manifestPath)) {
  $errors += "Missing governance manifest: $manifestPath"
} else {
  try {
    $manifest = Get-Content -Raw -Path $manifestPath | ConvertFrom-Json
  } catch {
    $errors += "Governance manifest is not valid JSON: $_"
  }

  if ($manifest) {
    foreach ($field in @("featureId", "schema", "docs", "tests", "owner")) {
      if (-not $manifest.$field) {
        $errors += "Governance manifest is missing '$field'"
      }
    }

    if ($manifest.schema -and -not (Test-Path (Join-Path $repoRoot ([string]$manifest.schema)))) {
      $errors += "Governance schema path does not exist: $($manifest.schema)"
    }
    if ($manifest.docs -and -not (Test-Path (Join-Path $repoRoot ([string]$manifest.docs)))) {
      $errors += "Governance docs path does not exist: $($manifest.docs)"
    }
    if ($manifest.tests -and -not (Test-Path (Join-Path $repoRoot ([string]$manifest.tests)))) {
      $errors += "Governance tests path does not exist: $($manifest.tests)"
    }
  }
}

$result = @{
  passed = $errors.Count -eq 0
  fixtureRoot = (Resolve-Path $FixtureRoot).Path
  errors = $errors
}

$result | ConvertTo-Json -Depth 4

if ($ExpectFailure) {
  if ($errors.Count -eq 0) {
    exit 1
  }
  exit 0
}

if ($errors.Count -gt 0) {
  exit 1
}
