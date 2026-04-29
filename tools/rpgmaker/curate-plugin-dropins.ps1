param(
    [string]$RepoRoot = "",
    [string]$SourcePluginRoot = "",
    [string]$CuratedPluginRoot = "",
    [switch]$CleanOutput
)

$ErrorActionPreference = "Stop"

function Ensure-Directory {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Read-TextSafe {
    param([string]$Path)
    try {
        return [System.IO.File]::ReadAllText($Path)
    } catch {
        return $null
    }
}

function Get-FileSha256 {
    param([string]$Path)
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try {
        $stream = [System.IO.File]::OpenRead($Path)
        try {
            $hash = $sha.ComputeHash($stream)
        } finally {
            $stream.Dispose()
        }
    } finally {
        $sha.Dispose()
    }
    return ([System.BitConverter]::ToString($hash)).Replace("-", "").ToLowerInvariant()
}

function Get-RelativePathNormalized {
    param(
        [string]$BasePath,
        [string]$TargetPath
    )

    $baseFull = [System.IO.Path]::GetFullPath($BasePath)
    $targetFull = [System.IO.Path]::GetFullPath($TargetPath)

    if ([System.IO.Path].GetMethod("GetRelativePath", [Type[]]@([string], [string])) -ne $null) {
        return [System.IO.Path]::GetRelativePath($baseFull, $targetFull).Replace("\", "/")
    }

    $baseUri = [System.Uri]::new(($baseFull.TrimEnd("\", "/") + [System.IO.Path]::DirectorySeparatorChar))
    $targetUri = [System.Uri]::new($targetFull)
    return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($targetUri).ToString()).Replace("\", "/")
}

function Get-SelectionScore {
    param([string]$PathRel)
    $p = $PathRel.ToLowerInvariant()
    $score = 0
    if ($p -match "sample_en|sample_project_en|mz example - en") { $score += 100 }
    if ($p -match "sample_ja|sample_jp|sample_project_jp|samplemap_ja|mz example - jp") { $score -= 20 }
    if ($p -match "menu builder") { $score += 15 }
    if ($p -match "samplemap_en|mv trinity") { $score -= 5 }
    $depthPenalty = ($PathRel.Split("/", [System.StringSplitOptions]::RemoveEmptyEntries).Count)
    $score -= $depthPenalty
    return $score
}

function Get-CandidateClassification {
    param(
        [string]$CandidatePathRel,
        [string]$CandidateSha256,
        [string]$SelectedSha256,
        [string]$ConflictType
    )

    $p = $CandidatePathRel.ToLowerInvariant()
    if ($p -match "sample_ja|sample_jp|sample_project_jp|samplemap_ja|mz example - jp") {
        return "localized_sample_copy"
    }
    if ($CandidateSha256 -eq $SelectedSha256 -or $ConflictType -eq "duplicate-copy") {
        return "identical_copy"
    }
    return "conflicting_plugin_implementation"
}

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
} else {
    $RepoRoot = (Resolve-Path $RepoRoot).Path
}

if ([string]::IsNullOrWhiteSpace($SourcePluginRoot)) {
    $SourcePluginRoot = Join-Path $RepoRoot "imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/plugin-dropins/js/plugins"
} elseif (-not [System.IO.Path]::IsPathRooted($SourcePluginRoot)) {
    $SourcePluginRoot = Join-Path $RepoRoot $SourcePluginRoot
}

if ([string]::IsNullOrWhiteSpace($CuratedPluginRoot)) {
    $CuratedPluginRoot = Join-Path $RepoRoot "imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/plugin-dropins-curated/js/plugins"
} elseif (-not [System.IO.Path]::IsPathRooted($CuratedPluginRoot)) {
    $CuratedPluginRoot = Join-Path $RepoRoot $CuratedPluginRoot
}

if (-not (Test-Path -LiteralPath $SourcePluginRoot)) {
    throw "Source plugin root not found: $SourcePluginRoot"
}

$sourceRootResolved = (Resolve-Path $SourcePluginRoot).Path
Ensure-Directory -Path $CuratedPluginRoot

if ($CleanOutput -and (Test-Path -LiteralPath $CuratedPluginRoot)) {
    Get-ChildItem -LiteralPath $CuratedPluginRoot -File -Filter "*.js" -ErrorAction SilentlyContinue | Remove-Item -Force
}

$reportRoot = Join-Path $RepoRoot "imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports"
Ensure-Directory -Path $reportRoot

$files = Get-ChildItem -LiteralPath $sourceRootResolved -Recurse -File -Filter "*.js" | Sort-Object FullName
if ($files.Count -eq 0) {
    throw "No JS plugin files found under: $sourceRootResolved"
}

$candidateRows = New-Object System.Collections.Generic.List[object]
foreach ($file in $files) {
    $pathRel = Get-RelativePathNormalized -BasePath $RepoRoot -TargetPath $file.FullName
    $text = Read-TextSafe -Path $file.FullName
    if ($null -eq $text) { continue }

    $stem = $file.BaseName
    $registerMatches = [regex]::Matches($text, "PluginManager\.registerCommand\(\s*['""]([^'""]+)['""]")
    $parameterMatches = [regex]::Matches($text, "PluginManager\.parameters\(\s*['""]([^'""]+)['""]")

    $pluginKeys = New-Object System.Collections.Generic.List[string]
    foreach ($m in $registerMatches) { $pluginKeys.Add($m.Groups[1].Value) }
    foreach ($m in $parameterMatches) { $pluginKeys.Add($m.Groups[1].Value) }
    $pluginKey = $stem
    if ($pluginKeys.Count -gt 0) {
        $pluginKey = ($pluginKeys | Select-Object -First 1)
    }

    $sha256 = Get-FileSha256 -Path $file.FullName
    $score = Get-SelectionScore -PathRel $pathRel

    $candidateRows.Add([PSCustomObject]@{
            PluginKey = $pluginKey
            PluginKeyLower = $pluginKey.ToLowerInvariant()
            Stem = $stem
            FileName = $file.Name
            PathRel = $pathRel
            FullPath = $file.FullName
            Sha256 = $sha256
            SizeBytes = [int64]$file.Length
            Score = $score
        })
}

$manifestRows = New-Object System.Collections.Generic.List[object]
$conflictRows = New-Object System.Collections.Generic.List[object]
$selectedKeys = New-Object 'System.Collections.Generic.HashSet[string]'

$groups = $candidateRows | Group-Object PluginKeyLower | Sort-Object Name
foreach ($g in $groups) {
    $cands = @($g.Group)
    $selected = $cands | Sort-Object @{ Expression = "Score"; Descending = $true }, @{ Expression = { $_.PathRel.Length }; Descending = $false }, @{ Expression = "PathRel"; Descending = $false } | Select-Object -First 1

    $outName = $selected.FileName
    $outPath = Join-Path $CuratedPluginRoot $outName
    if (-not $selectedKeys.Add($outName.ToLowerInvariant())) {
        $outName = "{0}__{1}.js" -f $selected.PluginKey, $selected.Sha256.Substring(0, 8)
        $outPath = Join-Path $CuratedPluginRoot $outName
        $null = $selectedKeys.Add($outName.ToLowerInvariant())
    }

    Copy-Item -LiteralPath $selected.FullPath -Destination $outPath -Force
    $outRel = Get-RelativePathNormalized -BasePath $RepoRoot -TargetPath $outPath

    $hashCount = (($cands | Select-Object -ExpandProperty Sha256 | Sort-Object -Unique).Count)
    $conflictType = "single"
    if ($cands.Count -gt 1 -and $hashCount -eq 1) { $conflictType = "duplicate-copy" }
    if ($cands.Count -gt 1 -and $hashCount -gt 1) { $conflictType = "conflict" }

    $manifestRows.Add([PSCustomObject]@{
            PluginKey = $selected.PluginKey
            OutputFile = $outName
            OutputPathRel = $outRel
            CandidateCount = $cands.Count
            UniqueHashCount = $hashCount
            ConflictType = $conflictType
            SelectedSourcePathRel = $selected.PathRel
            SelectedSha256 = $selected.Sha256
            SelectionScore = $selected.Score
        })

    foreach ($c in $cands) {
        $conflictRows.Add([PSCustomObject]@{
                PluginKey = $selected.PluginKey
                CandidatePathRel = $c.PathRel
                CandidateSha256 = $c.Sha256
                CandidateScore = $c.Score
                Selected = ($c.PathRel -eq $selected.PathRel)
                ConflictType = $conflictType
            })
    }
}

$summary = [PSCustomObject]@{
    GeneratedAt = (Get-Date -Format "yyyy-MM-ddTHH:mm:sszzz")
    RepoRoot = $RepoRoot
    SourcePluginRoot = $sourceRootResolved
    CuratedPluginRoot = $CuratedPluginRoot
    SourceFileCount = $files.Count
    CuratedFileCount = $manifestRows.Count
    ConflictKeyCount = (($manifestRows | Where-Object { $_.ConflictType -eq "conflict" }).Count)
    DuplicateCopyKeyCount = (($manifestRows | Where-Object { $_.ConflictType -eq "duplicate-copy" }).Count)
    SingleKeyCount = (($manifestRows | Where-Object { $_.ConflictType -eq "single" }).Count)
}

$summaryJsonPath = Join-Path $reportRoot "plugin_dropins_curated_summary.json"
$manifestCsvPath = Join-Path $reportRoot "plugin_dropins_curated_manifest.csv"
$conflictsCsvPath = Join-Path $reportRoot "plugin_dropins_curated_conflicts.csv"
$releaseManifestJsonPath = Join-Path $reportRoot "plugin_dropins_release_manifest.json"

$summary | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $summaryJsonPath
$manifestRows | Sort-Object PluginKey | Export-Csv -LiteralPath $manifestCsvPath -NoTypeInformation -Encoding UTF8
$conflictRows | Sort-Object PluginKey, CandidatePathRel | Export-Csv -LiteralPath $conflictsCsvPath -NoTypeInformation -Encoding UTF8

$manifestByKey = @{}
foreach ($row in $manifestRows) {
    $manifestByKey[$row.PluginKey.ToLowerInvariant()] = $row
}

$includedRows = foreach ($row in ($manifestRows | Sort-Object PluginKey)) {
    [PSCustomObject]@{
        pluginKey = $row.PluginKey
        outputFile = $row.OutputFile
        releasePath = $row.OutputPathRel
        selectedSourcePath = $row.SelectedSourcePathRel
        sha256 = $row.SelectedSha256
        candidateCount = [int]$row.CandidateCount
        uniqueHashCount = [int]$row.UniqueHashCount
        conflictType = $row.ConflictType
        classification = if ($row.ConflictType -eq "single") { "single_source" } else { "selected_release_candidate" }
        selectionScore = [int]$row.SelectionScore
    }
}

$excludedRows = foreach ($row in ($conflictRows | Where-Object { $_.Selected -ne $true } | Sort-Object PluginKey, CandidatePathRel)) {
    $selected = $manifestByKey[$row.PluginKey.ToLowerInvariant()]
    $classification = Get-CandidateClassification `
        -CandidatePathRel $row.CandidatePathRel `
        -CandidateSha256 $row.CandidateSha256 `
        -SelectedSha256 $selected.SelectedSha256 `
        -ConflictType $row.ConflictType
    [PSCustomObject]@{
        pluginKey = $row.PluginKey
        candidatePath = $row.CandidatePathRel
        candidateSha256 = $row.CandidateSha256
        candidateScore = [int]$row.CandidateScore
        selectedSourcePath = $selected.SelectedSourcePathRel
        selectedSha256 = $selected.SelectedSha256
        classification = $classification
        notPackagedReason = "excluded_$classification"
    }
}

$releaseManifest = [PSCustomObject]@{
    format = "URPG_RPGMAKER_PLUGIN_RELEASE_MANIFEST_V1"
    generatedAt = (Get-Date -Format "yyyy-MM-ddTHH:mm:sszzz")
    sourcePluginRoot = $sourceRootResolved
    releasePluginRoot = $CuratedPluginRoot
    selectionPolicy = @(
        "group_by_plugin_key",
        "prefer_english_sample_sources",
        "prefer_menu_builder_pack_when_scores_tie",
        "prefer_shorter_stable_repo_path",
        "preserve_all_raw_vendor_sources_outside_release_manifest"
    )
    included = @($includedRows)
    excluded = @($excludedRows)
}
$releaseManifest | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $releaseManifestJsonPath

Write-Output "SOURCE_ROOT`t$sourceRootResolved"
Write-Output "CURATED_ROOT`t$CuratedPluginRoot"
Write-Output "SOURCE_FILES`t$($files.Count)"
Write-Output "CURATED_FILES`t$($manifestRows.Count)"
Write-Output "CONFLICT_KEYS`t$($summary.ConflictKeyCount)"
Write-Output "DUPLICATE_COPY_KEYS`t$($summary.DuplicateCopyKeyCount)"
Write-Output "SUMMARY_JSON`t$summaryJsonPath"
Write-Output "MANIFEST_CSV`t$manifestCsvPath"
Write-Output "CONFLICTS_CSV`t$conflictsCsvPath"
Write-Output "RELEASE_MANIFEST_JSON`t$releaseManifestJsonPath"
