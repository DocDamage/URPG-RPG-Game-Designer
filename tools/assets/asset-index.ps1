param(
    [switch]$Force,
    [string[]]$Roots
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$script = Join-Path $repoRoot "tools\assets\asset_db.py"

$args = @($script, "index")
if ($Force) { $args += "--force" }
if ($Roots -and $Roots.Count -gt 0) {
    $args += "--roots"
    $args += $Roots
}

python @args
