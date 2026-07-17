$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$path = Join-Path $repoRoot "content\readiness\wave_e_qualification_matrix.json"
if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Wave E qualification matrix is missing: $path" }
$matrix = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
if ($matrix.schema -ne "urpg.wave_e_qualification_matrix.v1") { throw "Unsupported Wave E qualification schema." }

$expectedCounts = @{
  stressScenarios = 8; faultClasses = 9; securitySurfaces = 8; sanitizerLanes = 4; fuzzBoundaries = 4;
  privacyDefaults = 7; sbomComponentClasses = 5; platformCapabilities = 13; compatibilityDomains = 6;
  packageManifestFields = 7
}
foreach ($field in $expectedCounts.Keys) {
  $values = @($matrix.$field)
  if ($values.Count -ne $expectedCounts[$field] -or @($values | Select-Object -Unique).Count -ne $values.Count) {
    throw "Wave E '$field' coverage is incomplete or duplicated."
  }
}
if ($matrix.releaseTargets.web -ne "unsupported" -or $matrix.promotedAssetSelection.default -ne "none" -or
    $matrix.promotedAssetSelection.unsafeIdBehavior -ne "reject" -or
    $matrix.promotedAssetSelection.missingManifestBehavior -ne "reject") {
  throw "Wave E target or promoted-asset selection policy is unsafe."
}
Write-Host "Wave E qualification matrix passed: stress=8, faults=9, security=8, platform=13, packageFields=7."
