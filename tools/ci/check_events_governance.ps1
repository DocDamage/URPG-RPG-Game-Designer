param(
  [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$schemaPath = Join-Path $Root "content\schemas\events.schema.json"
$fixturePath = Join-Path $Root "content\fixtures\events_fixture.json"

foreach ($path in @($schemaPath, $fixturePath)) {
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
    throw "Missing event authoring governance artifact: $path"
  }

  try {
    Get-Content -LiteralPath $path -Raw | ConvertFrom-Json | Out-Null
  } catch {
    throw "Invalid JSON in event authoring governance artifact: $path"
  }
}

$schema = Get-Content -LiteralPath $schemaPath -Raw | ConvertFrom-Json
$fixture = Get-Content -LiteralPath $fixturePath -Raw | ConvertFrom-Json

if ($schema.title -ne "URPG Event Authoring Document") {
  throw "events.schema.json title drifted from the canonical event authoring schema title"
}

if (-not $fixture.maps -or -not $fixture.events) {
  throw "events_fixture.json must include at least one map and one event"
}

$unsupportedCommands = @($fixture.events |
  ForEach-Object { $_.pages } |
  ForEach-Object { $_.commands } |
  Where-Object { $_.kind -eq "unsupported" })

if ($unsupportedCommands.Count -eq 0) {
  throw "events_fixture.json must preserve at least one unsupported command fallback sample"
}

foreach ($command in $unsupportedCommands) {
  $fallback = $command.PSObject.Properties["_compat_command_fallbacks"]
  if ($null -eq $fallback) {
    throw "Unsupported command '$($command.id)' is missing _compat_command_fallbacks"
  }
}

Write-Host "Event authoring governance artifacts are present and structurally valid."
