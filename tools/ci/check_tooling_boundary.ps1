<#
.SYNOPSIS
    S33-T06: Tooling boundary enforcement guard.

.DESCRIPTION
    Scans engine/ and runtimes/ C++ source files to ensure that no tooling
    code from tools/retrieval/, tools/vision/, or tools/audio/ is imported
    or included directly by engine or runtime code.

    The engine runtime must consume only exported artifacts from those tools
    (JSON bundles, schema-validated manifests, etc.) — never the tool Python
    modules or implementation files.

    Exit codes:
      0 — No violations found.  Boundary is clean.
      1 — One or more violations found (or script error).

.PARAMETER RepoRoot
    Optional.  Path to the repository root.  Defaults to two levels above
    this script's directory.

.PARAMETER ReportPath
    Optional.  If provided, write a JSON violation report to this path.

.PARAMETER Strict
    If set, any warning (e.g. suspicious comment mentioning a tool path) also
    triggers a non-zero exit code.

.EXAMPLE
    # Run from repo root
    .\tools\ci\check_tooling_boundary.ps1

    # Save a report for CI artefact upload
    .\tools\ci\check_tooling_boundary.ps1 -ReportPath .\build\tooling_boundary_report.json
#>

Param(
    [string]$RepoRoot = "",
    [string]$ReportPath = "",
    [switch]$Strict
)

$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Resolve repo root
# ---------------------------------------------------------------------------
if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
}

$engineRoot  = Join-Path $RepoRoot "engine"
$runtimeRoot = Join-Path $RepoRoot "runtimes"

if (-not (Test-Path $engineRoot)) {
    Write-Error "engine/ directory not found at: $engineRoot"
    exit 1
}
if (-not (Test-Path $runtimeRoot)) {
    Write-Error "runtimes/ directory not found at: $runtimeRoot"
    exit 1
}

# ---------------------------------------------------------------------------
# Forbidden import patterns
# Any #include or Python/JS import referencing these tool sub-trees is a
# violation of the artifact boundary.
# ---------------------------------------------------------------------------
$forbiddenPatterns = @(
    [regex]::new('(?i)#\s*include\s+[<"].*tools[\\/]retrieval'),
    [regex]::new('(?i)#\s*include\s+[<"].*tools[\\/]vision'),
    [regex]::new('(?i)#\s*include\s+[<"].*tools[\\/]audio'),
    [regex]::new('(?i)\bimport\b.*tools[\\/]retrieval'),
    [regex]::new('(?i)\bimport\b.*tools[\\/]vision'),
    [regex]::new('(?i)\bimport\b.*tools[\\/]audio'),
    [regex]::new('(?i)\brequire\b\s*\(.*tools[\\/]retrieval'),
    [regex]::new('(?i)\brequire\b\s*\(.*tools[\\/]vision'),
    [regex]::new('(?i)\brequire\b\s*\(.*tools[\\/]audio')
)

# Patterns that warrant a warning (in comments or strings — not code violations
# but suspicious enough to flag when running in strict mode).
$warningPatterns = @(
    [regex]::new('(?i)//.*tools[\\/](retrieval|vision|audio)'),
    [regex]::new('(?i)/\*.*tools[\\/](retrieval|vision|audio)'),
    [regex]::new('(?i)"tools[\\/](retrieval|vision|audio)')
)

# File extensions to scan
$scanExtensions = @("*.cpp", "*.h", "*.hpp", "*.cc", "*.cxx", "*.inl")

# ---------------------------------------------------------------------------
# Scan helper
# ---------------------------------------------------------------------------
$violations = [System.Collections.Generic.List[hashtable]]::new()
$warnings   = [System.Collections.Generic.List[hashtable]]::new()

function Invoke-ScanDirectory {
    param([string]$Dir)

    $files = Get-ChildItem -Path $Dir -Recurse -Include $scanExtensions -File -ErrorAction SilentlyContinue
    foreach ($file in $files) {
        $relPath = $file.FullName.Substring($RepoRoot.ToString().Length).TrimStart('\', '/')
        $lines = Get-Content -LiteralPath $file.FullName -ErrorAction SilentlyContinue
        if ($null -eq $lines) { continue }

        $lineNumber = 0
        foreach ($line in $lines) {
            $lineNumber++
            foreach ($pattern in $forbiddenPatterns) {
                if ($pattern.IsMatch($line)) {
                    $violations.Add(@{
                        file    = $relPath
                        line    = $lineNumber
                        content = $line.Trim()
                        rule    = $pattern.ToString()
                        level   = "violation"
                    })
                }
            }

            if ($Strict) {
                foreach ($pattern in $warningPatterns) {
                    if ($pattern.IsMatch($line)) {
                        $warnings.Add(@{
                            file    = $relPath
                            line    = $lineNumber
                            content = $line.Trim()
                            rule    = $pattern.ToString()
                            level   = "warning"
                        })
                    }
                }
            }
        }
    }
}

# ---------------------------------------------------------------------------
# Run scan on engine/ and runtimes/
# ---------------------------------------------------------------------------
Write-Host "Checking tooling boundary in engine/ ..."
Invoke-ScanDirectory -Dir $engineRoot

Write-Host "Checking tooling boundary in runtimes/ ..."
Invoke-ScanDirectory -Dir $runtimeRoot

# ---------------------------------------------------------------------------
# Build report
# ---------------------------------------------------------------------------
$timestamp = [System.DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")

$report = [ordered]@{
    schema     = "content/schemas/ci_boundary_report.schema.json"
    tool       = "check_tooling_boundary"
    generated  = $timestamp
    clean      = ($violations.Count -eq 0)
    violation_count = $violations.Count
    warning_count   = $warnings.Count
    violations = $violations
    warnings   = $warnings
}

$reportJson = $report | ConvertTo-Json -Depth 10

# ---------------------------------------------------------------------------
# Optionally persist the report
# ---------------------------------------------------------------------------
if (-not [string]::IsNullOrWhiteSpace($ReportPath)) {
    $reportDir = Split-Path $ReportPath -Parent
    if (-not (Test-Path $reportDir)) {
        New-Item -ItemType Directory -Path $reportDir -Force | Out-Null
    }
    Set-Content -Path $ReportPath -Value $reportJson -Encoding UTF8
    Write-Host "Boundary report written to: $ReportPath"
}

# ---------------------------------------------------------------------------
# Print summary and exit
# ---------------------------------------------------------------------------
if ($violations.Count -eq 0) {
    Write-Host "Tooling boundary check PASSED. ($($warnings.Count) warning(s))"
    if ($Strict -and $warnings.Count -gt 0) {
        Write-Host ""
        Write-Host "Strict mode: warnings treated as errors."
        foreach ($w in $warnings) {
            Write-Host "  [WARN] $($w.file):$($w.line)  $($w.content)"
        }
        exit 1
    }
    exit 0
}

Write-Host ""
Write-Host "Tooling boundary check FAILED. $($violations.Count) violation(s) found:"
Write-Host ""
foreach ($v in $violations) {
    Write-Host "  [VIOLATION] $($v.file):$($v.line)"
    Write-Host "              $($v.content)"
    Write-Host ""
}

Write-Host "The engine/ and runtimes/ trees must not import tooling source code."
Write-Host "Consume exported artifacts (JSON bundles, schema-validated manifests) instead."
Write-Host "See docs/adr/ for the artifact boundary ADR and examples."
exit 1
