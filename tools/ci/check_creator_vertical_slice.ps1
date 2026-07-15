param(
  [string]$BuildDirectory = "build/dev-ninja-debug"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$acceptancePath = Join-Path $root "content\examples\creator_vertical_slice\acceptance.json"
$reportPath = Join-Path $BuildDirectory "creator_vertical_slice_report.json"

if (-not (Test-Path -LiteralPath $acceptancePath -PathType Leaf)) {
  throw "Creator vertical-slice acceptance manifest is missing: $acceptancePath"
}
if (-not (Test-Path -LiteralPath $reportPath -PathType Leaf)) {
  throw "Creator vertical-slice report is missing. Run the 'creator vertical slice' integration test first: $reportPath"
}

$acceptance = Get-Content -LiteralPath $acceptancePath -Raw | ConvertFrom-Json
$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
if ($acceptance.schema -ne "urpg.creator_vertical_slice.v1" -or $report.schema -ne "urpg.creator_vertical_slice_report.v1") {
  throw "Creator vertical-slice schema is unsupported."
}
if ($report.status -ne "passed") { throw "Creator vertical slice did not pass: $($report.status)" }

$expected = @($acceptance.required_runtime_steps)
$actual = @($report.completed_steps)
if ($expected.Count -ne $actual.Count -or @($expected | Where-Object { $_ -notin $actual }).Count -ne 0) {
  throw "Creator vertical-slice report is missing a required runtime step."
}
foreach ($forbidden in @('://', '.urpg/recovery', '.urpg/playtest', 'asset_catalog.db')) {
  if ($report.package_inventory -match [regex]::Escape($forbidden)) {
    throw "Creator vertical-slice package inventory contains forbidden path marker: $forbidden"
  }
}
Write-Host "Creator vertical-slice gate passed: $reportPath"
