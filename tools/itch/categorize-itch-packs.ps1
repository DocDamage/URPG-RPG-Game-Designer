param(
    [string]$PackRoot = ".\imports\raw\third_party_assets\itch-assets\packs",
    [string]$OutputRoot = ".\imports\raw\third_party_assets\itch-assets\packs-by-category",
    [string]$ReportRoot = ".\imports\raw\third_party_assets\itch-assets\reports",
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

function Classify-Pack {
    param(
        [string]$Name,
        [int]$TotalFiles,
        [int]$PngFiles,
        [int]$AudioFiles
    )
    $n = $Name.ToLowerInvariant()
    $checks = [ordered]@{
        "audio" = @("audio", "sfx", "sound", "music", "bgm")
        "ui" = @("ui", "hud", "interface", "menu")
        "icons" = @("icon")
        "mecha-scifi" = @("mecha", "robot", "xmodern", "scifi", "cyber")
        "monsters-creatures" = @("monster", "beholder", "undead", "wolf", "beast", "spider", "dragon")
        "characters-npcs" = @("character", "char", "npc", "head", "hat", "tribe", "cultist", "fighter", "patronbundle")
        "tilesets-world" = @("tileset", "tile", "terrain", "world", "map", "island", "seabed", "land", "forest", "beach", "dungeon", "ruin", "winter", "cloud", "atlantis", "jungle", "ashlands", "tower")
        "items-weapons-tools" = @("weapon", "shield", "wand", "tool", "attack")
        "effects-vfx" = @("weather", "effect", "animation", "fx")
    }

    if ($AudioFiles -gt 0) {
        return [PSCustomObject]@{ Category = "audio"; Reason = "contains audio files" }
    }
    if ($n -match "bundle|mega pack" -and $TotalFiles -ge 300) {
        return [PSCustomObject]@{ Category = "bundles-mega"; Reason = "large bundle/mega pack" }
    }

    foreach ($category in $checks.Keys) {
        foreach ($needle in $checks[$category]) {
            if ($n -like "*$needle*") {
                return [PSCustomObject]@{ Category = $category; Reason = "name contains '$needle'" }
            }
        }
    }

    if ($PngFiles -gt 0 -and $TotalFiles -le 20) {
        return [PSCustomObject]@{ Category = "small-sprites"; Reason = "small sprite-oriented pack" }
    }

    return [PSCustomObject]@{ Category = "misc"; Reason = "no strong keyword match" }
}

$packRootResolved = (Resolve-Path $PackRoot).Path
$outputRootResolved = (Resolve-Path (Split-Path -Parent $OutputRoot)).Path
$outputRootResolved = Join-Path $outputRootResolved (Split-Path -Leaf $OutputRoot)
$reportRootResolved = (Resolve-Path (Split-Path -Parent $ReportRoot)).Path
$reportRootResolved = Join-Path $reportRootResolved (Split-Path -Leaf $ReportRoot)

Ensure-Directory -Path $reportRootResolved
if (-not $DryRun) {
    Remove-IfExists -Path $outputRootResolved
}
Ensure-Directory -Path $outputRootResolved

$packs = Get-ChildItem -LiteralPath $packRootResolved -Directory | Sort-Object Name
$rows = New-Object System.Collections.Generic.List[object]

foreach ($pack in $packs) {
    $files = @(Get-ChildItem -LiteralPath $pack.FullName -Recurse -File -ErrorAction SilentlyContinue)
    $totalFiles = $files.Count
    $pngFiles = @($files | Where-Object { $_.Extension -ieq ".png" }).Count
    $audioFiles = @($files | Where-Object { $_.Extension -in @(".ogg", ".wav", ".mp3", ".m4a", ".flac") }).Count

    $classification = Classify-Pack -Name $pack.Name -TotalFiles $totalFiles -PngFiles $pngFiles -AudioFiles $audioFiles
    $category = $classification.Category
    $reason = $classification.Reason

    $catDir = Join-Path $outputRootResolved $category
    $linkPath = Join-Path $catDir $pack.Name
    Ensure-Directory -Path $catDir

    if (-not $DryRun) {
        New-Item -ItemType Junction -Path $linkPath -Target $pack.FullName | Out-Null
    }

    $rows.Add([PSCustomObject]@{
            Pack = $pack.Name
            Category = $category
            Reason = $reason
            Files = $totalFiles
            PngFiles = $pngFiles
            AudioFiles = $audioFiles
            SourcePath = $pack.FullName
            CategoryPath = $linkPath
        })
}

$csvPath = Join-Path $reportRootResolved "packs_semantic_categories.csv"
$jsonPath = Join-Path $reportRootResolved "packs_semantic_categories.json"
$summaryPath = Join-Path $reportRootResolved "packs_semantic_summary.csv"

$summaryRows = $rows | Group-Object Category | Sort-Object Name | ForEach-Object {
    [PSCustomObject]@{
        Category = $_.Name
        PackCount = $_.Count
        FileCount = ($_.Group | Measure-Object -Property Files -Sum).Sum
    }
}

if (-not $DryRun) {
    $rows | Export-Csv -LiteralPath $csvPath -NoTypeInformation -Encoding UTF8
    $rows | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $jsonPath
    $summaryRows | Export-Csv -LiteralPath $summaryPath -NoTypeInformation -Encoding UTF8
}

Write-Output "PACK_ROOT`t$packRootResolved"
Write-Output "OUTPUT_ROOT`t$outputRootResolved"
Write-Output "PACK_COUNT`t$($rows.Count)"
Write-Output "CATEGORY_COUNT`t$($summaryRows.Count)"
Write-Output "REPORT_CSV`t$csvPath"
Write-Output "REPORT_JSON`t$jsonPath"
Write-Output "REPORT_SUMMARY`t$summaryPath"
$summaryRows | Format-Table -AutoSize
