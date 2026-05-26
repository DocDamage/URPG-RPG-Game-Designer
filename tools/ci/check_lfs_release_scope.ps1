$ErrorActionPreference = "Stop"

$lfs = git lfs ls-files --name-only | Where-Object { $_ -ne "" }
$releaseRequired = Get-Content content/fixtures/project_governance_fixture.json -Raw | ConvertFrom-Json
$requiredPaths = @()
foreach ($asset in $releaseRequired.releaseAssets.assets) {
    if ($asset.path) {
        $requiredPaths += $asset.path
    }
}

$releaseLfs = $lfs | Where-Object { $requiredPaths -contains $_ }
if ($releaseLfs.Count -gt 0) {
    Write-Error ("Release-required assets are still LFS-tracked:`n" + ($releaseLfs | Out-String))
}

Write-Host "Release-required assets are not LFS-tracked. Total LFS paths in checkout: $($lfs.Count)"
