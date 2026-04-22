param(
    [Parameter(Mandatory = $true)]
    [string]$EnvFile,

    [string[]]$AiderArgs = @()
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$aiderExe = Join-Path $env:USERPROFILE ".local\bin\aider.exe"
$envPath = Join-Path $repoRoot $EnvFile

if (-not (Test-Path $aiderExe)) {
    throw "Aider executable not found: $aiderExe"
}

if (-not (Test-Path $envPath)) {
    $examplePath = "$envPath.example"
    Write-Host "Missing env file: $envPath" -ForegroundColor Red
    if (Test-Path $examplePath) {
        Write-Host "Copy the example and fill in your key(s):" -ForegroundColor Yellow
        Write-Host "  Copy-Item '$examplePath' '$envPath'" -ForegroundColor Yellow
    }
    exit 1
}

Push-Location $repoRoot
try {
    & $aiderExe --config .aider.conf.yml --env-file $envPath @AiderArgs
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
