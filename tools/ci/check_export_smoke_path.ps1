$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$buildRoot = Join-Path $repoRoot "build"

if (-not (Test-Path $buildRoot)) {
    throw "Missing build directory: $buildRoot. Build a local profile before running check_export_smoke_path.ps1."
}

$matches = Get-ChildItem -Path $buildRoot -Recurse -File | Where-Object {
    $_.BaseName -eq "urpg_tests" -and ($_.Extension -eq ".exe" -or [string]::IsNullOrEmpty($_.Extension))
} | Sort-Object -Property @(
    @{ Expression = "LastWriteTime"; Descending = $true },
    @{ Expression = "FullName"; Descending = $false }
)

if (-not $matches) {
    throw "Could not find urpg_tests under $buildRoot. Build a local profile before running check_export_smoke_path.ps1."
}

$testExecutable = ($matches | Select-Object -First 1).FullName
$buildDir = Split-Path $testExecutable -Parent
$sdlRuntimeDir = Join-Path $buildDir "_deps\sdl2-build"

if (Test-Path $sdlRuntimeDir) {
    $env:PATH = "$sdlRuntimeDir;$env:PATH"
}

& $testExecutable "[export][packager][real_smoke]" --reporter compact
if ($LASTEXITCODE -ne 0) {
    throw "Export smoke path tests failed with exit code $LASTEXITCODE."
}

Write-Host "Export smoke path gate passed."
