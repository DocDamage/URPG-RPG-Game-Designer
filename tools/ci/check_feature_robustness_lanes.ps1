param(
    [string]$Python = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")

if ([string]::IsNullOrWhiteSpace($Python)) {
    $pyLauncher = Get-Command py -ErrorAction SilentlyContinue
    if ($null -ne $pyLauncher) {
        $Python = "py -3"
    } else {
        $Python = "python"
    }
}

$pythonParts = $Python -split "\s+"
$scriptPath = Join-Path $repoRoot "tools\ci\check_feature_robustness_lanes.py"
$command = @($pythonParts + @($scriptPath))

Push-Location $repoRoot
try {
    & $command[0] $command[1..($command.Length - 1)]
    if ($LASTEXITCODE -ne 0) {
        throw "Feature robustness lane validation failed with exit code $LASTEXITCODE."
    }
} finally {
    Pop-Location
}
