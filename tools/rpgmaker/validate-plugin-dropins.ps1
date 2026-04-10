param(
    [string]$RepoRoot = "",
    [string]$PluginRoot = "",
    [string]$ReportPrefix = "plugin_dropins_validation",
    [switch]$FailOnError
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

function Add-Issue {
    param(
        [System.Collections.Generic.List[object]]$IssueRows,
        [string]$Severity,
        [string]$Code,
        [string]$PathRel,
        [string]$Message
    )
    $IssueRows.Add([PSCustomObject]@{
            Severity = $Severity
            Code = $Code
            PathRel = $PathRel
            Message = $Message
        })
}

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
} else {
    $RepoRoot = (Resolve-Path $RepoRoot).Path
}

if ([string]::IsNullOrWhiteSpace($PluginRoot)) {
    $PluginRoot = Join-Path $RepoRoot "third_party/rpgmaker-mz/steam-dlc/plugin-dropins/js/plugins"
} elseif (-not [System.IO.Path]::IsPathRooted($PluginRoot)) {
    $PluginRoot = Join-Path $RepoRoot $PluginRoot
}

if (-not (Test-Path -LiteralPath $PluginRoot)) {
    throw "Plugin root not found: $PluginRoot"
}

$pluginRootResolved = (Resolve-Path $PluginRoot).Path
$reportRoot = Join-Path $RepoRoot "third_party/rpgmaker-mz/steam-dlc/reports"
Ensure-Directory -Path $reportRoot

$files = Get-ChildItem -LiteralPath $pluginRootResolved -File -Recurse -Filter "*.js" | Sort-Object FullName
$fileRows = New-Object System.Collections.Generic.List[object]
$issueRows = New-Object System.Collections.Generic.List[object]

if ($files.Count -eq 0) {
    Add-Issue -IssueRows $issueRows -Severity "ERROR" -Code "no_js_files" -PathRel "" -Message "No plugin JS files found."
}

foreach ($file in $files) {
    $pathRel = Get-RelativePathNormalized -BasePath $RepoRoot -TargetPath $file.FullName
    $text = Read-TextSafe -Path $file.FullName
    if ($null -eq $text) {
        Add-Issue -IssueRows $issueRows -Severity "ERROR" -Code "read_failed" -PathRel $pathRel -Message "Unable to read file."
        continue
    }

    $size = [int64]$file.Length
    $stem = $file.BaseName
    $hasTargetMz = $text -match "@target\s+MZ"
    $hasPluginDesc = $text -match "@plugindesc\b"
    $registerMatches = [regex]::Matches($text, "PluginManager\.registerCommand\(\s*['""]([^'""]+)['""]")
    $parameterMatches = [regex]::Matches($text, "PluginManager\.parameters\(\s*['""]([^'""]+)['""]")

    $pluginKeys = New-Object System.Collections.Generic.List[string]
    foreach ($m in $registerMatches) { $pluginKeys.Add($m.Groups[1].Value) }
    foreach ($m in $parameterMatches) { $pluginKeys.Add($m.Groups[1].Value) }
    $pluginKey = $stem
    if ($pluginKeys.Count -gt 0) {
        $pluginKey = ($pluginKeys | Select-Object -First 1)
    }

    if ($size -eq 0) {
        Add-Issue -IssueRows $issueRows -Severity "ERROR" -Code "empty_file" -PathRel $pathRel -Message "File is zero bytes."
    }
    if (-not $hasTargetMz) {
        Add-Issue -IssueRows $issueRows -Severity "WARN" -Code "missing_target_mz" -PathRel $pathRel -Message "Missing @target MZ header."
    }
    if (-not $hasPluginDesc) {
        Add-Issue -IssueRows $issueRows -Severity "WARN" -Code "missing_plugindesc" -PathRel $pathRel -Message "Missing @plugindesc header."
    }

    $sha256 = Get-FileSha256 -Path $file.FullName

    $fileRows.Add([PSCustomObject]@{
            PathRel = $pathRel
            FileName = $file.Name
            Stem = $stem
            PluginKey = $pluginKey
            Sha256 = $sha256
            SizeBytes = $size
            HasTargetMZ = $hasTargetMz
            HasPluginDesc = $hasPluginDesc
            RegisterCommandCount = $registerMatches.Count
            ParametersCallCount = $parameterMatches.Count
        })
}

$stemGroups = $fileRows | Group-Object { $_.Stem.ToLowerInvariant() } | Where-Object { $_.Count -gt 1 }
foreach ($g in $stemGroups) {
    $paths = ($g.Group | ForEach-Object { $_.PathRel }) -join "; "
    $hashCount = (($g.Group | Select-Object -ExpandProperty Sha256 | Sort-Object -Unique).Count)
    foreach ($row in $g.Group) {
        if ($hashCount -gt 1) {
            Add-Issue -IssueRows $issueRows -Severity "ERROR" -Code "duplicate_stem_conflict" -PathRel $row.PathRel -Message "Conflicting plugin stem '$($row.Stem)': $paths"
        } else {
            Add-Issue -IssueRows $issueRows -Severity "WARN" -Code "duplicate_stem_copy" -PathRel $row.PathRel -Message "Duplicate copy of plugin stem '$($row.Stem)': $paths"
        }
    }
}

$keyGroups = $fileRows | Group-Object { $_.PluginKey.ToLowerInvariant() } | Where-Object { $_.Count -gt 1 }
foreach ($g in $keyGroups) {
    $paths = ($g.Group | ForEach-Object { $_.PathRel }) -join "; "
    $hashCount = (($g.Group | Select-Object -ExpandProperty Sha256 | Sort-Object -Unique).Count)
    foreach ($row in $g.Group) {
        if ($hashCount -gt 1) {
            Add-Issue -IssueRows $issueRows -Severity "ERROR" -Code "duplicate_plugin_key_conflict" -PathRel $row.PathRel -Message "Conflicting plugin key '$($row.PluginKey)': $paths"
        } else {
            Add-Issue -IssueRows $issueRows -Severity "WARN" -Code "duplicate_plugin_key_copy" -PathRel $row.PathRel -Message "Duplicate copy of plugin key '$($row.PluginKey)': $paths"
        }
    }
}

$errorCount = ($issueRows | Where-Object { $_.Severity -eq "ERROR" }).Count
$warnCount = ($issueRows | Where-Object { $_.Severity -eq "WARN" }).Count
$summary = [PSCustomObject]@{
    GeneratedAt = (Get-Date -Format "yyyy-MM-ddTHH:mm:sszzz")
    RepoRoot = $RepoRoot
    PluginRoot = $pluginRootResolved
    FileCount = $fileRows.Count
    ErrorCount = $errorCount
    WarnCount = $warnCount
    StemCollisionCount = $stemGroups.Count
    PluginKeyCollisionCount = $keyGroups.Count
}

$summaryJsonPath = Join-Path $reportRoot "${ReportPrefix}_summary.json"
$filesCsvPath = Join-Path $reportRoot "${ReportPrefix}_files.csv"
$issuesCsvPath = Join-Path $reportRoot "${ReportPrefix}_issues.csv"

$summary | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $summaryJsonPath
$fileRows | Export-Csv -LiteralPath $filesCsvPath -NoTypeInformation -Encoding UTF8
$issueRows | Export-Csv -LiteralPath $issuesCsvPath -NoTypeInformation -Encoding UTF8

Write-Output "REPO_ROOT`t$RepoRoot"
Write-Output "PLUGIN_ROOT`t$pluginRootResolved"
Write-Output "FILE_COUNT`t$($fileRows.Count)"
Write-Output "WARN_COUNT`t$warnCount"
Write-Output "ERROR_COUNT`t$errorCount"
Write-Output "REPORT_SUMMARY_JSON`t$summaryJsonPath"
Write-Output "REPORT_FILES_CSV`t$filesCsvPath"
Write-Output "REPORT_ISSUES_CSV`t$issuesCsvPath"

if ($FailOnError -and $errorCount -gt 0) {
    throw "Plugin drop-in validation found $errorCount error(s)."
}
