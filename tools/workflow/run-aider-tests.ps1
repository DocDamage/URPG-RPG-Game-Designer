param(
    [string]$Filter = ""
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$buildDir = Join-Path $repoRoot "build-local"
$testExe = Join-Path $buildDir "Debug\\urpg_tests.exe"

Push-Location $repoRoot
try {
    Write-Host "[aider-test] Building urpg_tests..."
    & cmake --build $buildDir --config Debug --target urpg_tests
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    if (-not (Test-Path $testExe)) {
        throw "Test executable not found: $testExe"
    }

    $args = @()
    if ($Filter) {
        $args += $Filter
    }

    Write-Host "[aider-test] Running urpg_tests $Filter"
    & $testExe @args
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
