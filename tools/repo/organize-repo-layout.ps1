param(
    [string]$RepoRoot = "",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

function Ensure-Directory {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Move-SafeFile {
    param(
        [string]$SourcePath,
        [string]$DestinationDir
    )
    if (-not (Test-Path -LiteralPath $SourcePath)) {
        return $null
    }
    Ensure-Directory -Path $DestinationDir
    $fileName = Split-Path -Path $SourcePath -Leaf
    $destPath = Join-Path $DestinationDir $fileName
    if ($DryRun) {
        return [PSCustomObject]@{ Source = $SourcePath; Destination = $destPath; Action = "DRYRUN" }
    }
    if (Test-Path -LiteralPath $destPath) {
        Remove-Item -LiteralPath $destPath -Force
    }
    Move-Item -LiteralPath $SourcePath -Destination $destPath -Force
    return [PSCustomObject]@{ Source = $SourcePath; Destination = $destPath; Action = "MOVED" }
}

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path ".").Path
} else {
    $RepoRoot = (Resolve-Path $RepoRoot).Path
}

$rootImports = Join-Path $RepoRoot "imports\root-drop"
$rootArchiveBase = Join-Path $rootImports "archives"

$targets = @(
    @{ Pattern = "CGMZ*.zip"; Dest = (Join-Path $rootArchiveBase "rpgmaker\cgmz") },
    @{ Pattern = "VisuMZ*.zip"; Dest = (Join-Path $rootArchiveBase "rpgmaker\visustella") },
    @{ Pattern = "AseGit*.zip"; Dest = (Join-Path $rootArchiveBase "aseprite") },
    @{ Pattern = "*ui*.zip"; Dest = (Join-Path $rootArchiveBase "ui") }
)

$moves = New-Object System.Collections.Generic.List[object]
foreach ($t in $targets) {
    $files = Get-ChildItem -LiteralPath $RepoRoot -File -Filter $t.Pattern -ErrorAction SilentlyContinue
    foreach ($file in $files) {
        $m = Move-SafeFile -SourcePath $file.FullName -DestinationDir $t.Dest
        if ($m) { $moves.Add($m) }
    }
}

# Organize itch root files into subfolders.
$itchRoot = Join-Path $RepoRoot "itch"
$itchArchives = Join-Path $itchRoot "archives"
$itchLoose = Join-Path $itchRoot "loose"
$itchPartials = Join-Path $itchRoot "partials"
Ensure-Directory -Path $itchRoot
Ensure-Directory -Path $itchArchives
Ensure-Directory -Path $itchLoose
Ensure-Directory -Path $itchPartials

$itchRootFiles = @()
if (Test-Path -LiteralPath $itchRoot) {
    $itchRootFiles = Get-ChildItem -LiteralPath $itchRoot -File -ErrorAction SilentlyContinue
}

foreach ($file in $itchRootFiles) {
    $dest = $itchLoose
    if ($file.Extension -ieq ".zip") {
        $dest = $itchArchives
    } elseif ($file.Extension -ieq ".crdownload") {
        $dest = $itchPartials
    }
    $m = Move-SafeFile -SourcePath $file.FullName -DestinationDir $dest
    if ($m) { $moves.Add($m) }
}

$reportDir = Join-Path $RepoRoot "imports\reports"
Ensure-Directory -Path $reportDir
$reportCsv = Join-Path $reportDir "repo_organize_moves.csv"
if (-not $DryRun) {
    $moves | Export-Csv -LiteralPath $reportCsv -NoTypeInformation -Encoding UTF8
}

Write-Output "REPO_ROOT`t$RepoRoot"
Write-Output "MOVE_COUNT`t$($moves.Count)"
Write-Output "REPORT`t$reportCsv"
