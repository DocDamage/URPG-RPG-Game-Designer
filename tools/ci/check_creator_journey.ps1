param(
  [string]$BuildDirectory = "build/dev-ninja-debug"
)

$ErrorActionPreference = "Stop"

$reportPath = Join-Path $BuildDirectory "creator_journey_report.json"
if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
  throw "Creator-journey report is missing. Run the 'creator journey' integration test first: $reportPath"
}

$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
if ($report.schema -ne "urpg.creator_journey_report.v1") {
  throw "Creator-journey report schema is unsupported: $($report.schema)"
}

$specPath = Join-Path $PSScriptRoot "..\..\content\fixtures\creator_journey_spec.json"
$spec = Get-Content -LiteralPath $specPath -Raw | ConvertFrom-Json
$expectedIds = @($spec.steps | ForEach-Object { $_.id })
$actualIds = @($report.steps | ForEach-Object { $_.id })

if ($actualIds.Count -ne $expectedIds.Count -or @($expectedIds | Where-Object { $_ -notin $actualIds }).Count -ne 0) {
  throw "Creator-journey report is missing one or more required step IDs."
}

foreach ($step in $report.steps) {
  if ($step.status -eq "failed") {
    throw "Creator-journey step '$($step.id)' failed: $($step.diagnostic_codes -join ', ')"
  }
  if ($step.status -eq "deferred") {
    $declared = $spec.steps | Where-Object { $_.id -eq $step.id } | Select-Object -First 1
    if ($null -eq $declared -or $declared.baseline_status -ne "deferred" -or [string]::IsNullOrWhiteSpace($declared.reason)) {
      throw "Creator-journey step '$($step.id)' is deferred without an explicit baseline declaration and remediation."
    }
  }
}

Write-Host "Creator-journey baseline passed: $reportPath"
