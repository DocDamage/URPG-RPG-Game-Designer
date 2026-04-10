param(
    [string]$ProjectRoot = ".",
    [switch]$Embed,
    [string[]]$IncludePattern = @(),
    [string[]]$ExcludePattern = @(
        ".git/*",
        "node_modules/*",
        "build/*",
        "dist/*",
        ".venv/*",
        "venv/*",
        ".cache/*"
    ),
    [string]$OutFile = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = "."
}
$projectPath = (Resolve-Path -LiteralPath $ProjectRoot).Path

$globalIndexCmd = Join-Path $env:LOCALAPPDATA "Programs\rom-tools\bin\codemunch-index.cmd"
$globalIndexEmbedCmd = Join-Path $env:LOCALAPPDATA "Programs\rom-tools\bin\codemunch-index-embed.cmd"

if (-not (Test-Path -LiteralPath $globalIndexCmd)) {
    throw "Global codemunch index wrapper not found: $globalIndexCmd"
}

if ($PSBoundParameters.ContainsKey("IncludePattern") -and $IncludePattern.Count -gt 0) {
    Write-Warning "IncludePattern is managed by global wrapper defaults in this mode and will be ignored."
}
if ($PSBoundParameters.ContainsKey("ExcludePattern") -and $ExcludePattern.Count -gt 0) {
    Write-Warning "ExcludePattern is managed by global wrapper defaults in this mode and will be ignored."
}

if ($Embed) {
    if (-not (Test-Path -LiteralPath $globalIndexEmbedCmd)) {
        throw "Global codemunch embed wrapper not found: $globalIndexEmbedCmd"
    }
    $result = & $globalIndexEmbedCmd $projectPath
} else {
    $result = & $globalIndexCmd $projectPath
}

if (-not [string]::IsNullOrWhiteSpace($OutFile)) {
    $outResolved = if ([System.IO.Path]::IsPathRooted($OutFile)) {
        $OutFile
    } else {
        Join-Path $projectPath $OutFile
    }
    $outDir = Split-Path -Parent $outResolved
    if ($outDir -and -not (Test-Path -LiteralPath $outDir)) {
        New-Item -ItemType Directory -Path $outDir -Force | Out-Null
    }
    Set-Content -LiteralPath $outResolved -Value $result -Encoding UTF8
    Write-Output "Wrote: $outResolved"
}

$result
