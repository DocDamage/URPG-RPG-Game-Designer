param(
    [Parameter(Mandatory = $true)]
    [string]$ProcessName,
    [Parameter(Mandatory = $true)]
    [string]$BinaryPath,
    [int]$TimeoutMs = 5000
)

$ErrorActionPreference = "Stop"

function Resolve-NormalizedPath {
    param([Parameter(Mandatory = $true)][string]$PathValue)
    try {
        return [System.IO.Path]::GetFullPath($PathValue)
    } catch {
        return $PathValue
    }
}

$normalizedBinaryPath = Resolve-NormalizedPath -PathValue $BinaryPath

$candidates = Get-Process -Name $ProcessName -ErrorAction SilentlyContinue
foreach ($process in $candidates) {
    try {
        $processPath = $null
        try {
            $processPath = $process.MainModule.FileName
        } catch {
            $processPath = $null
        }

        $killProcess = $false
        if (-not $processPath) {
            $killProcess = $true
        } else {
            $normalizedProcessPath = Resolve-NormalizedPath -PathValue $processPath
            $killProcess = [string]::Equals(
                $normalizedProcessPath,
                $normalizedBinaryPath,
                [System.StringComparison]::OrdinalIgnoreCase
            )
        }

        if ($killProcess) {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        }
    } catch {
        # Do nothing and continue best-effort cleanup.
    }
}

if (-not (Test-Path -LiteralPath $BinaryPath)) {
    exit 0
}

$deadline = (Get-Date).AddMilliseconds([Math]::Max(0, $TimeoutMs))
while ((Get-Date) -lt $deadline) {
    try {
        $stream = [System.IO.File]::Open(
            $BinaryPath,
            [System.IO.FileMode]::Open,
            [System.IO.FileAccess]::ReadWrite,
            [System.IO.FileShare]::None
        )
        $stream.Dispose()
        exit 0
    } catch {
        Start-Sleep -Milliseconds 100
    }
}

Write-Warning "Timed out waiting for binary to unlock: $BinaryPath"
