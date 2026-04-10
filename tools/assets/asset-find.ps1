param(
    [string]$Query = "",
    [string]$Ext = "",
    [string]$Kind = "",
    [string]$Pack = "",
    [string]$Category = "",
    [int]$Limit = 50
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$script = Join-Path $repoRoot "tools\assets\asset_db.py"

$args = @($script, "find", "--limit", "$Limit")
if ($Query) { $args += @("--query", $Query) }
if ($Ext) { $args += @("--ext", $Ext) }
if ($Kind) { $args += @("--kind", $Kind) }
if ($Pack) { $args += @("--pack", $Pack) }
if ($Category) { $args += @("--category", $Category) }

python @args
