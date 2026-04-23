param(
  [string]$ReportPath,
  [switch]$Strict
)

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$fixturesDir = Join-Path $repoRoot "tests\compat\fixtures\plugins"
$schemaChangelogPath = Join-Path $repoRoot "docs\SCHEMA_CHANGELOG.md"
$readinessPath = Join-Path $repoRoot "content\readiness\readiness_status.json"

$issues = [System.Collections.Generic.List[hashtable]]::new()

function Add-Issue {
  param([string]$Category, [string]$Severity, [string]$Message)
  $issues.Add(@{ category = $Category; severity = $Severity; message = $Message })
}

# -----------------------------------------------------------------------
# 1. Corpus existence check
# -----------------------------------------------------------------------
if (-not (Test-Path $fixturesDir)) {
  Add-Issue "corpus" "ERROR" "Compat fixtures directory is missing: $fixturesDir"
}
else {
  $fixtureFiles = Get-ChildItem -Path $fixturesDir -Filter "*.json" -File
  if ($fixtureFiles.Count -eq 0) {
    Add-Issue "corpus" "ERROR" "No fixture JSON files found under $fixturesDir"
  }

  # -----------------------------------------------------------------------
  # 2. Per-fixture structural health checks
  # -----------------------------------------------------------------------
  foreach ($file in $fixtureFiles) {
    $raw = Get-Content -Raw -LiteralPath $file.FullName
    $parsed = $raw | ConvertFrom-Json -ErrorAction SilentlyContinue

    if ($null -eq $parsed) {
      Add-Issue "fixture_parse" "ERROR" "Fixture '$($file.Name)' is not valid JSON."
      continue
    }

    # Required top-level fields
    foreach ($field in @("name", "version", "commands")) {
      if (-not $parsed.PSObject.Properties.Name.Contains($field)) {
        Add-Issue "fixture_schema" "ERROR" "Fixture '$($file.Name)' is missing required field '$field'."
      }
    }

    # name must match filename stem
    $expectedName = [System.IO.Path]::GetFileNameWithoutExtension($file.Name)
    if ($parsed.name -and $parsed.name -ne $expectedName) {
      Add-Issue "fixture_naming" "WARN" "Fixture '$($file.Name)': 'name' field '$($parsed.name)' does not match filename stem '$expectedName'."
    }

    # commands array checks
    if ($parsed.commands -is [System.Array] -or $parsed.commands -is [System.Collections.Generic.List[object]]) {
      if ($parsed.commands.Count -eq 0) {
        Add-Issue "fixture_schema" "WARN" "Fixture '$($file.Name)' has an empty 'commands' array."
      }
      foreach ($cmd in $parsed.commands) {
        if (-not $cmd.PSObject.Properties.Name.Contains("name")) {
          Add-Issue "fixture_schema" "ERROR" "Fixture '$($file.Name)': a command entry is missing a 'name' field."
        }
        if (-not $cmd.PSObject.Properties.Name.Contains("script")) {
          Add-Issue "fixture_schema" "WARN" "Fixture '$($file.Name)': command '$($cmd.name)' is missing a 'script' field."
        }
      }
    }

    # -----------------------------------------------------------------------
    # 3. Dependency drift detection
    # -----------------------------------------------------------------------
    if ($parsed.PSObject.Properties.Name.Contains("dependencies") -and
        $parsed.dependencies -is [System.Array]) {
      $knownFixtureNames = $fixtureFiles | ForEach-Object {
        [System.IO.Path]::GetFileNameWithoutExtension($_.Name)
      }
      foreach ($dep in $parsed.dependencies) {
        if ($dep -notin $knownFixtureNames) {
          Add-Issue "dependency_drift" "WARN" "Fixture '$($file.Name)' declares dependency '$dep' which is not present in the curated corpus. This may indicate a dependency drift or a missing fixture."
        }
      }
    }

    # -----------------------------------------------------------------------
    # 4. Profile mismatch detection (basic convention check)
    # -----------------------------------------------------------------------
    if ($parsed.PSObject.Properties.Name.Contains("parameters") -and
        $parsed.parameters.PSObject.Properties.Name.Contains("profile")) {
      $profileValue = [string]$parsed.parameters.profile
      if ($profileValue -cmatch '[A-Z]{2,}' -and $profileValue -notmatch '^[A-Za-z]') {
        # Profile names should not be all-caps screaming identifiers
        Add-Issue "profile_mismatch" "WARN" "Fixture '$($file.Name)': 'parameters.profile' value '$profileValue' uses non-conventional uppercase naming. Verify this is intentional."
      }
    }
  }
}

# -----------------------------------------------------------------------
# 5. Schema/changelog validation for compat import lane (S26-T04)
# -----------------------------------------------------------------------
if (-not (Test-Path $schemaChangelogPath)) {
  Add-Issue "schema_changelog" "ERROR" "SCHEMA_CHANGELOG.md is missing: $schemaChangelogPath"
}
else {
  $changelogText = Get-Content -Raw -Path $schemaChangelogPath

  # Any compat-related schemas should be referenced in the changelog
  $compatSchemaKeywords = @("compat", "plugin", "battle_actions", "battle_troops")
  foreach ($keyword in $compatSchemaKeywords) {
    if ($changelogText -notmatch [regex]::Escape($keyword)) {
      Add-Issue "schema_changelog" "WARN" "SCHEMA_CHANGELOG.md does not reference '$keyword'. Confirm all compat-related schemas are changelog-tracked."
    }
  }
}

# -----------------------------------------------------------------------
# 6. Readiness record check: compat_bridge_exit must have signoff contract
# -----------------------------------------------------------------------
if (-not (Test-Path $readinessPath)) {
  Add-Issue "readiness" "ERROR" "readiness_status.json is missing: $readinessPath"
}
else {
  $readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
  $compatEntry = $readiness.subsystems | Where-Object { $_.id -eq "compat_bridge_exit" } | Select-Object -First 1
  if (-not $compatEntry) {
    Add-Issue "readiness" "ERROR" "readiness_status.json does not contain a 'compat_bridge_exit' subsystem entry."
  }
  elseif (-not $compatEntry.PSObject.Properties.Name.Contains("signoff")) {
    Add-Issue "readiness" "ERROR" "'compat_bridge_exit' is missing a structured signoff contract in readiness_status.json."
  }
  elseif ($compatEntry.signoff.required -ne $true) {
    Add-Issue "readiness" "ERROR" "'compat_bridge_exit' signoff.required is not true in readiness_status.json."
  }
}

# -----------------------------------------------------------------------
# Results
# -----------------------------------------------------------------------
$errorCount  = ($issues | Where-Object { $_.severity -eq "ERROR" }).Count
$warnCount   = ($issues | Where-Object { $_.severity -eq "WARN"  }).Count

if ($issues.Count -gt 0) {
  Write-Host ""
  Write-Host "== Compat corpus health check issues ==" -ForegroundColor Yellow
  foreach ($issue in $issues) {
    $color = if ($issue.severity -eq "ERROR") { "Red" } else { "Yellow" }
    Write-Host "  [$($issue.severity)] [$($issue.category)] $($issue.message)" -ForegroundColor $color
  }
  Write-Host ""
}

if ($ReportPath) {
  $report = @{
    timestamp   = (Get-Date -Format "yyyy-MM-ddTHH:mm:ssZ")
    passed      = ($errorCount -eq 0)
    errorCount  = $errorCount
    warnCount   = $warnCount
    issues      = $issues
  }
  $report | ConvertTo-Json -Depth 6 | Out-File -FilePath $ReportPath -Encoding utf8
  Write-Host "Compat health report written to: $ReportPath"
}

if ($errorCount -gt 0) {
  Write-Error "Compat corpus health check failed with $errorCount error(s) and $warnCount warning(s)."
  exit 1
}

if ($Strict -and $warnCount -gt 0) {
  Write-Error "Compat corpus health check failed in strict mode with $warnCount warning(s)."
  exit 1
}

Write-Host "Compat corpus health check passed. Errors: $errorCount  Warnings: $warnCount" -ForegroundColor Green
