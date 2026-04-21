param(
    [Parameter(Mandatory = $true)]
    [string]$TestName,

    [Parameter(Mandatory = $true)]
    [string]$SnapshotId,

    [Parameter(Mandatory = $true)]
    [string]$SourcePath,

    [string]$GoldenRoot = "tests/snapshot/goldens/"
)

$ErrorActionPreference = "Stop"

$destinationFileName = "{0}_{1}.golden.json" -f $TestName, $SnapshotId
$destinationPath = Join-Path $GoldenRoot $destinationFileName

$destinationDir = Split-Path $destinationPath -Parent
if (-not (Test-Path $destinationDir)) {
    New-Item -ItemType Directory -Path $destinationDir -Force | Out-Null
}

Copy-Item -Path $SourcePath -Destination $destinationPath -Force
Write-Host "Approved golden for ${TestName}_${SnapshotId}"
