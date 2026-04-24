param(
  [string]$SourceRoot = "more assets",
  [string]$ExtractRoot = "imports/raw/more_assets",
  [string]$ReportRoot = "imports/reports/more_assets",
  [switch]$InventoryOnly,
  [switch]$Force
)

$ErrorActionPreference = "Stop"

function Resolve-RepoPath([string]$PathValue) {
  if ([System.IO.Path]::IsPathRooted($PathValue)) {
    return [System.IO.Path]::GetFullPath($PathValue)
  }
  return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $PathValue))
}

function Convert-ToRelativePath([string]$BasePath, [string]$ChildPath) {
  $baseFull = [System.IO.Path]::GetFullPath($BasePath)
  if (-not $baseFull.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $baseFull += [System.IO.Path]::DirectorySeparatorChar
  }
  $childFull = [System.IO.Path]::GetFullPath($ChildPath)
  $baseUri = [System.Uri]::new($baseFull)
  $childUri = [System.Uri]::new($childFull)
  return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($childUri).ToString()).Replace('\', '/')
}

function Convert-ToSafeSlug([string]$Name) {
  $withoutExtension = [System.IO.Path]::GetFileNameWithoutExtension($Name)
  $slug = $withoutExtension.ToLowerInvariant() -replace '[^a-z0-9]+', '-'
  $slug = $slug.Trim('-')
  if ([string]::IsNullOrWhiteSpace($slug)) {
    return "archive"
  }
  return $slug
}

function Get-Sha256([string]$PathValue) {
  return (Get-FileHash -LiteralPath $PathValue -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Get-ArchiveKind([string]$PathValue) {
  $extension = [System.IO.Path]::GetExtension($PathValue).ToLowerInvariant()
  switch ($extension) {
    ".zip" { return "zip" }
    ".7z" { return "7z" }
    ".rar" { return "rar" }
    default { return "unsupported" }
  }
}

function Test-ZipEntrySafe([string]$DestinationRoot, [System.IO.Compression.ZipArchiveEntry]$Entry) {
  if ([string]::IsNullOrWhiteSpace($Entry.FullName)) {
    return $false
  }
  $entryName = $Entry.FullName.Replace('\', '/')
  if ($entryName.StartsWith("/") -or $entryName.Contains("../") -or $entryName.Contains("..\")) {
    return $false
  }
  $targetPath = [System.IO.Path]::GetFullPath((Join-Path $DestinationRoot $Entry.FullName))
  $destinationFull = [System.IO.Path]::GetFullPath($DestinationRoot)
  return $targetPath.StartsWith($destinationFull, [System.StringComparison]::OrdinalIgnoreCase)
}

function Read-ZipInventory([string]$ArchivePath, [string]$RepoRoot, [string]$ArchiveExtractRoot) {
  $archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
  try {
    $entries = @()
    $unsafeCount = 0
    $licenseEntries = @()
    $readmeEntries = @()
    $totalUncompressedBytes = [int64]0

    foreach ($entry in $archive.Entries) {
      $isDirectory = [string]::IsNullOrEmpty($entry.Name)
      if (-not (Test-ZipEntrySafe -DestinationRoot $ArchiveExtractRoot -Entry $entry)) {
        $unsafeCount += 1
      }
      if (-not $isDirectory) {
        $totalUncompressedBytes += [int64]$entry.Length
        $lower = $entry.FullName.ToLowerInvariant()
        if ($lower -match '(^|/)(license|licence|copying|credits)(\.|_|-|$)' -or $lower -match 'license|licence') {
          $licenseEntries += $entry.FullName
        }
        if ($lower -match '(^|/)readme(\.|_|-|$)' -or $lower.EndsWith(".md")) {
          $readmeEntries += $entry.FullName
        }
        $entries += [ordered]@{
          path = $entry.FullName.Replace('\', '/')
          size_bytes = [int64]$entry.Length
          compressed_size_bytes = [int64]$entry.CompressedLength
        }
      }
    }

    return [ordered]@{
      entry_count = $archive.Entries.Count
      file_count = $entries.Count
      total_uncompressed_bytes = $totalUncompressedBytes
      unsafe_entry_count = $unsafeCount
      license_entries = $licenseEntries
      readme_entries = $readmeEntries
      entries = $entries
    }
  } finally {
    $archive.Dispose()
  }
}

function Expand-ZipSafely([string]$ArchivePath, [string]$DestinationRoot) {
  $archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
  try {
    foreach ($entry in $archive.Entries) {
      if ([string]::IsNullOrEmpty($entry.Name)) {
        continue
      }
      if (-not (Test-ZipEntrySafe -DestinationRoot $DestinationRoot -Entry $entry)) {
        throw "Unsafe zip entry '$($entry.FullName)' in '$ArchivePath'."
      }

      $targetPath = [System.IO.Path]::GetFullPath((Join-Path $DestinationRoot $entry.FullName))
      $targetDir = [System.IO.Path]::GetDirectoryName($targetPath)
      [System.IO.Directory]::CreateDirectory($targetDir) | Out-Null
      [System.IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $targetPath, $true)
    }
  } finally {
    $archive.Dispose()
  }
}

Add-Type -AssemblyName System.IO.Compression.FileSystem

$repoRoot = [System.IO.Path]::GetFullPath((Get-Location))
$sourceFull = Resolve-RepoPath $SourceRoot
$extractFull = Resolve-RepoPath $ExtractRoot
$reportFull = Resolve-RepoPath $ReportRoot

if (-not (Test-Path -LiteralPath $sourceFull -PathType Container)) {
  throw "Source root not found: $sourceFull"
}

[System.IO.Directory]::CreateDirectory($extractFull) | Out-Null
[System.IO.Directory]::CreateDirectory($reportFull) | Out-Null

$archiveExtensions = @(".zip", ".7z", ".rar")
$installerExtensions = @(".exe", ".msi")
$sourceFiles = Get-ChildItem -LiteralPath $sourceFull -Recurse -File
$archiveFiles = $sourceFiles |
  Where-Object { $archiveExtensions -contains $_.Extension.ToLowerInvariant() } |
  Sort-Object FullName
$installerFiles = $sourceFiles |
  Where-Object { $installerExtensions -contains $_.Extension.ToLowerInvariant() } |
  Sort-Object FullName

$archiveRecords = @()
$extractedArchiveCount = 0
$skippedArchiveCount = 0
$failedArchiveCount = 0

foreach ($archiveFile in $archiveFiles) {
  $relativeArchivePath = Convert-ToRelativePath -BasePath $repoRoot -ChildPath $archiveFile.FullName
  $kind = Get-ArchiveKind $archiveFile.FullName
  $slug = Convert-ToSafeSlug $archiveFile.Name
  $hash = Get-Sha256 $archiveFile.FullName
  $shortHash = $hash.Substring(0, 12)
  $archiveExtractRoot = Join-Path $extractFull "$slug-$shortHash"
  $archiveExtractRelative = Convert-ToRelativePath -BasePath $repoRoot -ChildPath $archiveExtractRoot

  $record = [ordered]@{
    source_archive = $relativeArchivePath
    archive_kind = $kind
    size_bytes = [int64]$archiveFile.Length
    sha256 = $hash
    extraction_root = $archiveExtractRelative
    extracted = $false
    skipped_reason = $null
    error = $null
    inventory = $null
  }

  if ($kind -ne "zip") {
    $record.skipped_reason = "unsupported_archive_kind"
    $skippedArchiveCount += 1
    $archiveRecords += $record
    continue
  }

  try {
    $record.inventory = Read-ZipInventory -ArchivePath $archiveFile.FullName -RepoRoot $repoRoot -ArchiveExtractRoot $archiveExtractRoot
    if ($record.inventory.unsafe_entry_count -gt 0) {
      $record.skipped_reason = "unsafe_zip_entries"
      $skippedArchiveCount += 1
      $archiveRecords += $record
      continue
    }

    if ($InventoryOnly) {
      $record.skipped_reason = "inventory_only"
      $skippedArchiveCount += 1
      $archiveRecords += $record
      continue
    }

    if ((Test-Path -LiteralPath $archiveExtractRoot) -and -not $Force) {
      $record.extracted = $true
      $record.skipped_reason = "already_extracted"
      $skippedArchiveCount += 1
      $archiveRecords += $record
      continue
    }

    [System.IO.Directory]::CreateDirectory($archiveExtractRoot) | Out-Null
    Expand-ZipSafely -ArchivePath $archiveFile.FullName -DestinationRoot $archiveExtractRoot
    $record.extracted = $true
    $extractedArchiveCount += 1
  } catch {
    $record.error = $_.Exception.Message
    $failedArchiveCount += 1
  }

  $archiveRecords += $record
}

$installerRecords = @()
foreach ($installerFile in $installerFiles) {
  $installerRecords += [ordered]@{
    path = Convert-ToRelativePath -BasePath $repoRoot -ChildPath $installerFile.FullName
    size_bytes = [int64]$installerFile.Length
    sha256 = Get-Sha256 $installerFile.FullName
    skipped_reason = "installer_not_executed"
  }
}

$totalArchiveBytes = [int64]0
$presentExtractedArchiveCount = 0
foreach ($record in $archiveRecords) {
  $totalArchiveBytes += [int64]$record.size_bytes
  if ($record.extracted) {
    $presentExtractedArchiveCount += 1
  }
}

$manifest = [ordered]@{
  report_version = 1
  generated_at = (Get-Date).ToUniversalTime().ToString("o")
  source_root = Convert-ToRelativePath -BasePath $repoRoot -ChildPath $sourceFull
  extract_root = Convert-ToRelativePath -BasePath $repoRoot -ChildPath $extractFull
  summary = [ordered]@{
    archive_count = $archiveRecords.Count
    zip_archive_count = ($archiveRecords | Where-Object { $_.archive_kind -eq "zip" }).Count
    unsupported_archive_count = ($archiveRecords | Where-Object { $_.archive_kind -ne "zip" }).Count
    extracted_archive_count = $extractedArchiveCount
    present_extracted_archive_count = $presentExtractedArchiveCount
    skipped_archive_count = $skippedArchiveCount
    failed_archive_count = $failedArchiveCount
    installer_count = $installerRecords.Count
    total_archive_bytes = $totalArchiveBytes
  }
  archives = $archiveRecords
  installers = $installerRecords
}

$manifestPath = Join-Path $reportFull "more_assets_intake_manifest.json"
$manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8

Write-Host "More-assets intake manifest: $(Convert-ToRelativePath -BasePath $repoRoot -ChildPath $manifestPath)"
Write-Host "Archives: $($manifest.summary.archive_count), extracted: $extractedArchiveCount, skipped: $skippedArchiveCount, failed: $failedArchiveCount, installers skipped: $($installerRecords.Count)"
