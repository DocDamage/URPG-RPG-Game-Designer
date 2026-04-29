param(
    [string]$SteamDlcRoot = "C:\Program Files (x86)\Steam\steamapps\common\RPG Maker MZ\dlc",
    [string]$RepoRoot = "",
    [switch]$DryRun,
    [switch]$SkipCurate,
    [switch]$SkipCuratedValidation
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

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
} else {
    $RepoRoot = (Resolve-Path $RepoRoot).Path
}

if (-not (Test-Path -LiteralPath $SteamDlcRoot)) {
    throw "Steam DLC path not found: $SteamDlcRoot"
}

$steamDlcRootResolved = (Resolve-Path $SteamDlcRoot).Path
$vendorRoot = Join-Path $RepoRoot "imports/raw/third_party_assets/rpgmaker-mz/steam-dlc"
$packsRoot = Join-Path $vendorRoot "packs"
$pluginRoot = Join-Path $vendorRoot "plugin-dropins/js/plugins/steam-dlc"
$reportRoot = Join-Path $vendorRoot "reports"

Ensure-Directory -Path $vendorRoot
Ensure-Directory -Path $packsRoot
Ensure-Directory -Path $pluginRoot
Ensure-Directory -Path $reportRoot

$packs = Get-ChildItem -LiteralPath $steamDlcRootResolved -Directory | Sort-Object Name
if ($packs.Count -eq 0) {
    throw "No DLC folders found in: $steamDlcRootResolved"
}

$pluginFileRows = New-Object System.Collections.Generic.List[object]
$packRows = New-Object System.Collections.Generic.List[object]

foreach ($pack in $packs) {
    $destPack = Join-Path $packsRoot $pack.Name
    $destPluginPack = Join-Path $pluginRoot $pack.Name

    if (-not $DryRun) {
        Remove-IfExists -Path $destPack
        Remove-IfExists -Path $destPluginPack
        Ensure-Directory -Path $destPack
        Ensure-Directory -Path $destPluginPack
        Copy-Item -LiteralPath $pack.FullName -Destination $destPack -Recurse -Force
    }

    $packFiles = Get-ChildItem -LiteralPath $pack.FullName -Recurse -File -ErrorAction SilentlyContinue
    $jsFiles = $packFiles | Where-Object { $_.Extension -ieq ".js" }
    $pluginJsFiles = $jsFiles | Where-Object {
        $rel = $_.FullName.Substring($pack.FullName.Length).TrimStart('\')
        $isRootJs = ((Split-Path -Path $rel -Parent) -eq "")
        $isPluginsFolderJs = ($rel -imatch '(^|\\)js\\plugins\\[^\\]+\.js$')
        return ($isRootJs -or $isPluginsFolderJs)
    }

    foreach ($js in $pluginJsFiles) {
        $relPath = $js.FullName.Substring($pack.FullName.Length).TrimStart('\')
        $stagePath = Join-Path $destPluginPack $relPath

        if (-not $DryRun) {
            $stageDir = Split-Path -Parent $stagePath
            Ensure-Directory -Path $stageDir
            Copy-Item -LiteralPath $js.FullName -Destination $stagePath -Force
        }

        $pluginFileRows.Add([PSCustomObject]@{
                Pack = $pack.Name
                FileName = $js.Name
                RelativePath = $relPath
                StagedPath = $stagePath
            })
    }

    $packRows.Add([PSCustomObject]@{
            Pack = $pack.Name
            SourcePath = $pack.FullName
            StagedPackPath = $destPack
            TotalFiles = $packFiles.Count
            JsFiles = $jsFiles.Count
            PluginJsFiles = $pluginJsFiles.Count
        })
}

$stamp = Get-Date -Format "yyyy-MM-ddTHH:mm:sszzz"
$summary = [PSCustomObject]@{
    GeneratedAt = $stamp
    RepoRoot = $RepoRoot
    SteamDlcRoot = $steamDlcRootResolved
    PackCount = $packRows.Count
    PluginJsCount = $pluginFileRows.Count
    Packs = $packRows
}

$summaryJsonPath = Join-Path $reportRoot "steam_mz_dlc_inventory.json"
$packCsvPath = Join-Path $reportRoot "steam_mz_dlc_packs.csv"
$pluginCsvPath = Join-Path $reportRoot "steam_mz_dlc_plugins.csv"

if (-not $DryRun) {
    $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $summaryJsonPath
    $packRows | Export-Csv -LiteralPath $packCsvPath -NoTypeInformation -Encoding UTF8
    $pluginFileRows | Export-Csv -LiteralPath $pluginCsvPath -NoTypeInformation -Encoding UTF8

    if (-not $SkipCurate) {
        & (Join-Path $PSScriptRoot "curate-plugin-dropins.ps1") -RepoRoot $RepoRoot -CleanOutput
    }

    if (-not $SkipCuratedValidation) {
        & (Join-Path $PSScriptRoot "validate-plugin-dropins.ps1") `
            -RepoRoot $RepoRoot `
            -PluginRoot "imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/plugin-dropins-curated/js/plugins" `
            -ReportPrefix "plugin_dropins_curated_validation" `
            -FailOnError
    }
}

Write-Output "REPO_ROOT`t$RepoRoot"
Write-Output "STEAM_DLC_ROOT`t$steamDlcRootResolved"
Write-Output "PACK_COUNT`t$($packRows.Count)"
Write-Output "PLUGIN_JS_COUNT`t$($pluginFileRows.Count)"
Write-Output "PACKS_ROOT`t$packsRoot"
Write-Output "PLUGIN_DROPINS`t$pluginRoot"
Write-Output "CURATED_PLUGIN_DROPINS`t$(Join-Path $vendorRoot "plugin-dropins-curated/js/plugins")"
Write-Output "REPORT_JSON`t$summaryJsonPath"
Write-Output "REPORT_PACKS_CSV`t$packCsvPath"
Write-Output "REPORT_PLUGINS_CSV`t$pluginCsvPath"
Write-Output "CURATE_SKIPPED`t$SkipCurate"
Write-Output "CURATED_VALIDATION_SKIPPED`t$SkipCuratedValidation"
