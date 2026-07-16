[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)]
  [string]$DestinationDirectory,

  [Parameter(Mandatory = $true, ValueFromRemainingArguments = $true)]
  [string[]]$SourceFiles
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Get-UrpgSha256 {
  param([Parameter(Mandatory = $true)][string]$Path)

  $stream = [System.IO.File]::Open(
    $Path,
    [System.IO.FileMode]::Open,
    [System.IO.FileAccess]::Read,
    [System.IO.FileShare]::ReadWrite
  )
  $hasher = [System.Security.Cryptography.SHA256]::Create()
  try {
    return (($hasher.ComputeHash($stream) | ForEach-Object { $_.ToString("x2") }) -join "")
  } finally {
    $hasher.Dispose()
    $stream.Dispose()
  }
}

if (-not (Test-Path -LiteralPath $DestinationDirectory -PathType Container)) {
  New-Item -ItemType Directory -Path $DestinationDirectory -Force | Out-Null
}

foreach ($sourceFile in $SourceFiles) {
  if ([string]::IsNullOrWhiteSpace($sourceFile)) {
    continue
  }
  if (-not (Test-Path -LiteralPath $sourceFile -PathType Leaf)) {
    throw "Runtime dependency does not exist: $sourceFile"
  }

  $destinationFile = Join-Path $DestinationDirectory (Split-Path $sourceFile -Leaf)
  if (Test-Path -LiteralPath $destinationFile -PathType Leaf) {
    $sourceHash = Get-UrpgSha256 -Path $sourceFile
    $destinationHash = Get-UrpgSha256 -Path $destinationFile
    if ($sourceHash -eq $destinationHash) {
      Write-Verbose "Runtime dependency is already current: $destinationFile"
      continue
    }
  }

  Copy-Item -LiteralPath $sourceFile -Destination $destinationFile -Force
}
