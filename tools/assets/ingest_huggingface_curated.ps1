[CmdletBinding()]
param(
    [string]$ManifestPath = ".\tools\assets\huggingface_curated_manifest.json",
    [switch]$Refresh,
    [string]$ReportPath = ".\imports\reports\huggingface_curated_inventory.json"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Convert-ToRepoRelativeUrlPath {
    param([Parameter(Mandatory)][string]$RelativePath)

    return (($RelativePath -split "/") | ForEach-Object {
        [System.Uri]::EscapeDataString($_)
    }) -join "/"
}

function New-HuggingFaceResolveUrl {
    param(
        [Parameter(Mandatory)][string]$Repo,
        [Parameter(Mandatory)][string]$RelativePath
    )

    $encodedPath = Convert-ToRepoRelativeUrlPath -RelativePath $RelativePath
    return "https://huggingface.co/datasets/$Repo/resolve/main/${encodedPath}?download=true"
}

$repoRoot = Resolve-Path "."
$manifestFullPath = Join-Path $repoRoot $ManifestPath
$reportFullPath = Join-Path $repoRoot $ReportPath

if (-not (Test-Path $manifestFullPath)) {
    throw "Manifest not found: $manifestFullPath"
}

$manifest = Get-Content $manifestFullPath -Raw | ConvertFrom-Json
$reportEntries = @()

foreach ($dataset in $manifest.datasets) {
    $targetRoot = Join-Path $repoRoot $dataset.targetRoot
    New-Item -ItemType Directory -Force -Path $targetRoot | Out-Null

    foreach ($relativePath in $dataset.files) {
        $targetPath = Join-Path $targetRoot ($relativePath -replace "/", [IO.Path]::DirectorySeparatorChar)
        $targetDir = Split-Path -Parent $targetPath
        New-Item -ItemType Directory -Force -Path $targetDir | Out-Null

        $entry = [ordered]@{
            repo = $dataset.repo
            license = $dataset.license
            ingestMode = $dataset.ingestMode
            relativePath = $relativePath
            targetPath = [IO.Path]::GetRelativePath($repoRoot, $targetPath)
            downloaded = $false
        }

        if ($dataset.ingestMode -eq "vendor_direct") {
            if ($Refresh -or -not (Test-Path $targetPath)) {
                $url = New-HuggingFaceResolveUrl -Repo $dataset.repo -RelativePath $relativePath
                Invoke-WebRequest -Uri $url -OutFile $targetPath
            }
            $entry.downloaded = Test-Path $targetPath
        }

        $reportEntries += [pscustomobject]$entry
    }
}

$reportDir = Split-Path -Parent $reportFullPath
New-Item -ItemType Directory -Force -Path $reportDir | Out-Null

$report = [ordered]@{
    generatedAtUtc = (Get-Date).ToUniversalTime().ToString("o")
    manifest = [IO.Path]::GetRelativePath($repoRoot, $manifestFullPath)
    datasets = $reportEntries
}

$report | ConvertTo-Json -Depth 6 | Set-Content -Path $reportFullPath
Write-Host "Wrote Hugging Face curated inventory to $reportFullPath"
