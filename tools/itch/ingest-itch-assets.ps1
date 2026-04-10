param(
    [string]$ItchRoot = ".\itch",
    [string]$OutputRoot = "",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

function Ensure-Directory {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Remove-IfExists {
    param([string]$Path)
    if (Test-Path -LiteralPath $Path) {
        Remove-Item -LiteralPath $Path -Recurse -Force
    }
}

function Get-SafeFolderName {
    param([string]$Name)
    $safe = $Name -replace '[<>:"/\\|?*]', "_"
    $safe = $safe -replace '\s+', ' '
    $safe = $safe.Trim()
    if ([string]::IsNullOrWhiteSpace($safe)) {
        $safe = "asset_pack"
    }
    if ($safe.Length -gt 100) {
        $safe = $safe.Substring(0, 100).Trim()
    }
    return $safe
}

function Get-UniqueFolderPath {
    param(
        [string]$Root,
        [string]$PreferredName
    )
    $candidate = Join-Path $Root $PreferredName
    if (-not (Test-Path -LiteralPath $candidate)) {
        return $candidate
    }
    $i = 2
    while ($true) {
        $candidate = Join-Path $Root ("{0} ({1})" -f $PreferredName, $i)
        if (-not (Test-Path -LiteralPath $candidate)) {
            return $candidate
        }
        $i++
    }
}

$repoRoot = (Resolve-Path ".").Path
$itchRootResolved = (Resolve-Path $ItchRoot).Path

if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $outputRootResolved = Join-Path $repoRoot "third_party\itch-assets"
} else {
    $outputRootResolved = (Resolve-Path $OutputRoot).Path
}

$packsRoot = Join-Path $outputRootResolved "packs"
$looseRoot = Join-Path $outputRootResolved "loose-files"
$reportsRoot = Join-Path $outputRootResolved "reports"

Ensure-Directory -Path $outputRootResolved
if (-not $DryRun) {
    Remove-IfExists -Path $packsRoot
    Remove-IfExists -Path $looseRoot
    Remove-IfExists -Path $reportsRoot
}
Ensure-Directory -Path $packsRoot
Ensure-Directory -Path $looseRoot
Ensure-Directory -Path $reportsRoot

$allFiles = Get-ChildItem -LiteralPath $itchRootResolved -Recurse -File |
    Where-Object {
        $_.FullName -notmatch '\\unzipped\\' -and
        $_.FullName -notmatch '\\partials\\'
    } |
    Sort-Object FullName
$archives = $allFiles | Where-Object { $_.Extension -ieq ".zip" }
$partials = $allFiles | Where-Object { $_.Extension -ieq ".crdownload" }
$loose = $allFiles | Where-Object { $_.Extension -ine ".zip" -and $_.Extension -ine ".crdownload" }

$archiveRows = New-Object System.Collections.Generic.List[object]
foreach ($archive in $archives) {
    $hash = Get-FileHash -LiteralPath $archive.FullName -Algorithm SHA256
    $archiveRows.Add([PSCustomObject]@{
            FileName = $archive.Name
            FullPath = $archive.FullName
            BaseName = [System.IO.Path]::GetFileNameWithoutExtension($archive.Name)
            LengthBytes = $archive.Length
            LastWriteTime = $archive.LastWriteTime
            Sha256 = $hash.Hash
        })
}

$dedupRows = New-Object System.Collections.Generic.List[object]
$packRows = New-Object System.Collections.Generic.List[object]
$extractErrors = New-Object System.Collections.Generic.List[object]

$groups = $archiveRows | Group-Object Sha256 | Sort-Object { $_.Group[0].FileName }

foreach ($group in $groups) {
    $ordered = $group.Group | Sort-Object FileName
    $canonical = $ordered | Select-Object -First 1
    $dupes = @($ordered | Select-Object -Skip 1)

    $preferredFolder = Get-SafeFolderName -Name $canonical.BaseName
    $packFolder = Get-UniqueFolderPath -Root $packsRoot -PreferredName $preferredFolder

    if (-not $DryRun) {
        Ensure-Directory -Path $packFolder
        try {
            Expand-Archive -LiteralPath $canonical.FullPath -DestinationPath $packFolder -Force
        } catch {
            $extractErrors.Add([PSCustomObject]@{
                    FileName = $canonical.FileName
                    FullPath = $canonical.FullPath
                    Error = $_.Exception.Message
                })
        }
    }

    $packRows.Add([PSCustomObject]@{
            CanonicalFile = $canonical.FileName
            CanonicalPath = $canonical.FullPath
            Sha256 = $canonical.Sha256
            DuplicateCount = $dupes.Count
            ExtractedTo = $packFolder
        })

    $dedupRows.Add([PSCustomObject]@{
            FileName = $canonical.FileName
            CanonicalFile = $canonical.FileName
            Sha256 = $canonical.Sha256
            IsDuplicate = $false
            ExtractedTo = $packFolder
        })

    foreach ($dup in $dupes) {
        $dedupRows.Add([PSCustomObject]@{
                FileName = $dup.FileName
                CanonicalFile = $canonical.FileName
                Sha256 = $dup.Sha256
                IsDuplicate = $true
                ExtractedTo = $packFolder
            })
    }
}

$looseRows = New-Object System.Collections.Generic.List[object]
foreach ($file in $loose) {
    $relative = $file.FullName.Substring($itchRootResolved.Length).TrimStart('\')
    $destPath = Join-Path $looseRoot $relative
    if (-not $DryRun) {
        $destDir = Split-Path -Parent $destPath
        Ensure-Directory -Path $destDir
        if (Test-Path -LiteralPath $destPath) {
            Remove-Item -LiteralPath $destPath -Force
        }
        Copy-Item -LiteralPath $file.FullName -Destination $destPath -Force
    }
    $looseRows.Add([PSCustomObject]@{
            FileName = $file.Name
            SourcePath = $file.FullName
            StagedPath = $destPath
            LengthBytes = $file.Length
        })
}

$packStatRows = New-Object System.Collections.Generic.List[object]
foreach ($pack in $packRows) {
    $files = @()
    if (Test-Path -LiteralPath $pack.ExtractedTo) {
        $files = @(Get-ChildItem -LiteralPath $pack.ExtractedTo -Recurse -File -ErrorAction SilentlyContinue)
    }
    $packStatRows.Add([PSCustomObject]@{
            CanonicalFile = $pack.CanonicalFile
            ExtractedTo = $pack.ExtractedTo
            TotalFiles = $files.Count
            PngFiles = (@($files | Where-Object { $_.Extension -ieq ".png" })).Count
            JsFiles = (@($files | Where-Object { $_.Extension -ieq ".js" })).Count
            JsonFiles = (@($files | Where-Object { $_.Extension -ieq ".json" })).Count
            AudioFiles = (@($files | Where-Object { $_.Extension -in @(".ogg", ".wav", ".mp3", ".m4a", ".flac") })).Count
        })
}

$summary = [PSCustomObject]@{
    GeneratedAt = (Get-Date -Format "yyyy-MM-ddTHH:mm:sszzz")
    RepoRoot = $repoRoot
    ItchRoot = $itchRootResolved
    OutputRoot = $outputRootResolved
    TotalFilesInItchRoot = $allFiles.Count
    ArchiveCount = $archives.Count
    UniqueArchiveCount = $groups.Count
    DuplicateArchiveCount = $dedupRows.Where({ $_.IsDuplicate }).Count
    PartialDownloadCount = $partials.Count
    LooseFileCount = $loose.Count
    ExtractErrorCount = $extractErrors.Count
}

$summaryJson = Join-Path $reportsRoot "summary.json"
$archiveInventoryCsv = Join-Path $reportsRoot "archive_inventory.csv"
$archiveDedupCsv = Join-Path $reportsRoot "archive_dedup.csv"
$packStatsCsv = Join-Path $reportsRoot "pack_stats.csv"
$partialsCsv = Join-Path $reportsRoot "partial_downloads.csv"
$looseCsv = Join-Path $reportsRoot "loose_files.csv"
$extractErrorsCsv = Join-Path $reportsRoot "extract_errors.csv"

if (-not $DryRun) {
    $summary | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $summaryJson
    $archiveRows | Export-Csv -LiteralPath $archiveInventoryCsv -NoTypeInformation -Encoding UTF8
    $dedupRows | Export-Csv -LiteralPath $archiveDedupCsv -NoTypeInformation -Encoding UTF8
    $packStatRows | Export-Csv -LiteralPath $packStatsCsv -NoTypeInformation -Encoding UTF8
    $partials | Select-Object Name, FullName, Length, LastWriteTime | Export-Csv -LiteralPath $partialsCsv -NoTypeInformation -Encoding UTF8
    $looseRows | Export-Csv -LiteralPath $looseCsv -NoTypeInformation -Encoding UTF8
    $extractErrors | Export-Csv -LiteralPath $extractErrorsCsv -NoTypeInformation -Encoding UTF8
}

Write-Output "ITCH_ROOT`t$itchRootResolved"
Write-Output "OUTPUT_ROOT`t$outputRootResolved"
Write-Output "TOTAL_FILES`t$($allFiles.Count)"
Write-Output "ARCHIVES`t$($archives.Count)"
Write-Output "UNIQUE_ARCHIVES`t$($groups.Count)"
Write-Output "DUPLICATE_ARCHIVES`t$($dedupRows.Where({ $_.IsDuplicate }).Count)"
Write-Output "PARTIAL_DOWNLOADS`t$($partials.Count)"
Write-Output "LOOSE_FILES`t$($loose.Count)"
Write-Output "EXTRACT_ERRORS`t$($extractErrors.Count)"
Write-Output "REPORT_SUMMARY`t$summaryJson"
