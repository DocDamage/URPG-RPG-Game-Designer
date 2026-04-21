$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$buildRoot = Join-Path $repoRoot "build"

if (-not (Test-Path $buildRoot)) {
    throw "Missing build directory: $buildRoot. Build the repo before running check_visual_regression_harness.ps1."
}

$matches = Get-ChildItem -Path $buildRoot -Recurse -File | Where-Object {
    $_.BaseName -eq "urpg_tests" -and ($_.Extension -eq ".exe" -or [string]::IsNullOrEmpty($_.Extension))
} | Sort-Object -Property @(
    @{ Expression = "LastWriteTime"; Descending = $true },
    @{ Expression = "FullName"; Descending = $false }
)

if (-not $matches) {
    throw "Could not find urpg_tests under $buildRoot. Build the repo before running check_visual_regression_harness.ps1."
}

$testExecutable = ($matches | Select-Object -First 1).FullName
& $testExecutable "[testing][visual_regression]" --reporter compact
if ($LASTEXITCODE -ne 0) {
    throw "Visual regression harness tests failed with exit code $LASTEXITCODE."
}

Write-Host "Visual regression harness gate passed."
