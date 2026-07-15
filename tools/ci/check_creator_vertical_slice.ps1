param(
  [string]$BuildDirectory = "build/dev-ninja-debug"
)

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$buildDirectory = if ([System.IO.Path]::IsPathRooted($BuildDirectory)) {
  [System.IO.Path]::GetFullPath($BuildDirectory)
} else {
  [System.IO.Path]::GetFullPath((Join-Path $repoRoot $BuildDirectory))
}
$acceptancePath = Join-Path $repoRoot "content\examples\creator_vertical_slice\acceptance.json"
$reportPath = Join-Path $buildDirectory "creator_vertical_slice_report.json"
foreach ($path in @($acceptancePath, $reportPath)) {
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
    throw "Creator vertical-slice input is missing: $path"
  }
}

$acceptance = Get-Content -LiteralPath $acceptancePath -Raw | ConvertFrom-Json
$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
if ($acceptance.schema -ne "urpg.creator_vertical_slice.v1") {
  throw "Unsupported creator vertical-slice acceptance schema: $($acceptance.schema)"
}
if ($report.schema -ne "urpg.creator_vertical_slice_report.v1" -or $report.status -ne "passed") {
  throw "Creator vertical slice has not produced a passed native report."
}
if ($report.id -ne $acceptance.id) {
  throw "Creator vertical-slice report ID does not match the governed acceptance manifest."
}
$expected = @($acceptance.required_runtime_steps | ForEach-Object { [string]$_ })
$actual = @($report.completed_steps | ForEach-Object { [string]$_ })
if ($expected.Count -ne $actual.Count -or @($expected | Where-Object { $_ -notin $actual }).Count -ne 0) {
  throw "Creator vertical-slice report is missing a required runtime step."
}
$inventory = @($report.package_inventory | ForEach-Object { [string]$_ })
foreach ($forbidden in @('://', '.urpg/recovery', '.urpg/playtest', 'asset_catalog.db')) {
  if (@($inventory | Where-Object { $_ -match [regex]::Escape($forbidden) }).Count -gt 0) {
    throw "Creator vertical-slice package inventory contains forbidden path marker: $forbidden"
  }
}
Write-Host "Creator vertical-slice gate passed: $reportPath"
