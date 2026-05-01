param(
    [string]$Python = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot
try {
    if ([string]::IsNullOrWhiteSpace($Python)) {
        $pyLauncher = Get-Command py -ErrorAction SilentlyContinue
        if ($null -ne $pyLauncher) {
            $Python = "py -3.13"
        } else {
            $Python = "python"
        }
    }

    $pythonParts = $Python -split "\s+"
    $command = @($pythonParts + @(
            "-m",
            "bandit",
            "-r",
            "tools/assets",
            "-x",
            "tools/assets/tests",
            "-ll"
        ))

    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $output = & $command[0] $command[1..($command.Length - 1)] 2>&1
        $exitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
    $output | ForEach-Object { Write-Host "$_" }

    if ($output -match "Files skipped \((?!0\))") {
        throw "Bandit skipped files. Run with a compatible Python/Bandit combination before accepting results."
    }
    if ($exitCode -ne 0) {
        throw "Bandit failed with exit code $exitCode."
    }
} finally {
    Pop-Location
}
