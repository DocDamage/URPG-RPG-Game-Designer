<#
.SYNOPSIS
    S25-T04: Template-subsystem-bar drift detection.

.DESCRIPTION
    For every template that has a canonical spec file at
    docs/templates/<template_id>_spec.md, this script verifies that:

    1. The spec file's "Required Subsystems" table lists the same subsystem IDs
       as content/readiness/readiness_status.json for that template.
    2. The spec file's "Cross-Cutting Minimum Bars" table lists status values
       that are compatible with the bars recorded in readiness_status.json.
    3. The spec file has a "Status Date" header that matches the readiness
       statusDate field.
    4. The spec file has the canonical "Authority:" line identifying the
       template it belongs to.

    This prevents the spec doc and the readiness record from drifting silently
    when one is updated and the other is not.

    Exit codes:
      0 — No drift detected.
      1 — One or more drift violations found (or script error).

.PARAMETER RepoRoot
    Optional.  Path to the repository root.  Defaults to two levels above
    this script's directory.

.PARAMETER ReportPath
    Optional.  If provided, write a JSON report to this path.

.EXAMPLE
    .\tools\ci\check_template_spec_bar_drift.ps1
    .\tools\ci\check_template_spec_bar_drift.ps1 -ReportPath .\build\template_bar_drift_report.json
#>

Param(
    [string]$RepoRoot = "",
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
}

$readinessPath = Join-Path $RepoRoot "content\readiness\readiness_status.json"
$templatesDir  = Join-Path $RepoRoot "docs\templates"

if (-not (Test-Path $readinessPath)) {
    Write-Error "Missing readiness file: $readinessPath"
    exit 1
}
if (-not (Test-Path $templatesDir)) {
    Write-Error "Missing templates directory: $templatesDir"
    exit 1
}

$readiness = Get-Content -Raw -Path $readinessPath | ConvertFrom-Json
$violations = [System.Collections.Generic.List[hashtable]]::new()

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

function Get-SpecStatusDate {
    param([string]$Text)
    $m = [regex]::Match($Text, 'Status Date:\s*(\d{4}-\d{2}-\d{2})')
    if ($m.Success) { return $m.Groups[1].Value }
    return $null
}

function Get-SpecAuthorityId {
    param([string]$Text)
    # Line like: Authority: canonical template spec for `<id>`
    $m = [regex]::Match($Text, 'Authority:\s+canonical template spec for `([^`]+)`')
    if ($m.Success) { return $m.Groups[1].Value }
    return $null
}

function Get-SpecRequiredSubsystems {
    param([string]$Text)
    # Parse the Required Subsystems table: lines like | `subsystem_id` | ...
    $ids = [System.Collections.Generic.List[string]]::new()
    $inTable = $false
    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match '##\s+Required Subsystems') {
            $inTable = $true
            continue
        }
        if ($inTable -and $line -match '^##') {
            break
        }
        if ($inTable -and $line -match '^\|\s+`([^`]+)`') {
            $ids.Add($matches[1])
        }
    }
    return $ids
}

function Get-SpecBarStatuses {
    param([string]$Text)
    # Parse the Cross-Cutting Minimum Bars table: lines like | Accessibility | `PARTIAL` | ...
    $bars = @{}
    $inTable = $false
    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match '##\s+Cross-Cutting Minimum Bars') {
            $inTable = $true
            continue
        }
        if ($inTable -and $line -match '^##') {
            break
        }
        if ($inTable -and $line -match '^\|\s+(\w+)\s+\|\s+`([^`]+)`') {
            $bars[$matches[1].ToLower()] = $matches[2]
        }
    }
    return $bars
}

# ---------------------------------------------------------------------------
# Check each template that has a canonical spec file
# ---------------------------------------------------------------------------
foreach ($template in $readiness.templates) {
    $specPath = Join-Path $templatesDir "$($template.id)_spec.md"
    if (-not (Test-Path $specPath)) {
        # No spec file yet — skip (S30-T05 will create missing specs)
        continue
    }

    $specText = Get-Content -Raw -Path $specPath

    # 1. Authority line
    $authorityId = Get-SpecAuthorityId -Text $specText
    if ([string]::IsNullOrWhiteSpace($authorityId)) {
        $violations.Add(@{
            template = $template.id
            rule     = "authority_missing"
            detail   = "Spec file '$($specPath | Split-Path -Leaf)' is missing the Authority line: 'Authority: canonical template spec for ``$($template.id)``'."
        })
    } elseif ($authorityId -ne $template.id) {
        $violations.Add(@{
            template = $template.id
            rule     = "authority_mismatch"
            detail   = "Spec authority ID '$authorityId' does not match template id '$($template.id)'."
        })
    }

    # 2. Status date
    $specDate = Get-SpecStatusDate -Text $specText
    if ([string]::IsNullOrWhiteSpace($specDate)) {
        $violations.Add(@{
            template = $template.id
            rule     = "status_date_missing"
            detail   = "Spec file '$($specPath | Split-Path -Leaf)' is missing a 'Status Date: YYYY-MM-DD' line."
        })
    } elseif ($specDate -ne $readiness.statusDate) {
        $violations.Add(@{
            template = $template.id
            rule     = "status_date_drift"
            detail   = "Spec '$($specPath | Split-Path -Leaf)' Status Date '$specDate' != readiness statusDate '$($readiness.statusDate)'. Update the spec after changing readiness_status.json."
        })
    }

    # 3. Required subsystems drift
    $specSubsystems = Get-SpecRequiredSubsystems -Text $specText
    if ($null -ne $template.requiredSubsystems) {
        $readinessSubsystems = @($template.requiredSubsystems)

        foreach ($id in $readinessSubsystems) {
            if ($specSubsystems -notcontains $id) {
                $violations.Add(@{
                    template = $template.id
                    rule     = "required_subsystem_missing_from_spec"
                    detail   = "Readiness record lists required subsystem '$id' for template '$($template.id)', but it is absent from the spec's Required Subsystems table."
                })
            }
        }

        foreach ($id in $specSubsystems) {
            if ($readinessSubsystems -notcontains $id) {
                $violations.Add(@{
                    template = $template.id
                    rule     = "extra_subsystem_in_spec"
                    detail   = "Spec lists required subsystem '$id' for template '$($template.id)', but it is absent from readiness_status.json.  Either add it to readiness or remove it from the spec."
                })
            }
        }
    }

    # 4. Bar status drift
    if ($null -ne $template.bars) {
        $specBars = Get-SpecBarStatuses -Text $specText
        # Convert readiness bars PSObject to hashtable
        $readinessBars = @{}
        foreach ($prop in $template.bars.PSObject.Properties) {
            $readinessBars[$prop.Name.ToLower()] = $prop.Value
        }

        foreach ($barName in $readinessBars.Keys) {
            $readinessStatus = $readinessBars[$barName]
            if ($specBars.ContainsKey($barName)) {
                $specStatus = $specBars[$barName]
                if ($specStatus -ne $readinessStatus) {
                    $violations.Add(@{
                        template = $template.id
                        rule     = "bar_status_drift"
                        detail   = "Bar '$barName' for template '$($template.id)': spec says '$specStatus', readiness says '$readinessStatus'. They must match."
                    })
                }
            } else {
                $violations.Add(@{
                    template = $template.id
                    rule     = "bar_missing_from_spec"
                    detail   = "Bar '$barName' is in readiness record for template '$($template.id)' but absent from the spec's Cross-Cutting Minimum Bars table."
                })
            }
        }
    }
}

# ---------------------------------------------------------------------------
# Build report
# ---------------------------------------------------------------------------
$timestamp = [System.DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
$report = [ordered]@{
    schema          = "content/schemas/ci_boundary_report.schema.json"
    tool            = "check_template_spec_bar_drift"
    generated       = $timestamp
    clean           = ($violations.Count -eq 0)
    violation_count = $violations.Count
    violations      = $violations
}

if (-not [string]::IsNullOrWhiteSpace($ReportPath)) {
    $reportDir = Split-Path $ReportPath -Parent
    if (-not (Test-Path $reportDir)) {
        New-Item -ItemType Directory -Path $reportDir -Force | Out-Null
    }
    Set-Content -Path $ReportPath -Value ($report | ConvertTo-Json -Depth 10) -Encoding UTF8
    Write-Host "Template spec bar drift report written to: $ReportPath"
}

# ---------------------------------------------------------------------------
# Output and exit
# ---------------------------------------------------------------------------
if ($violations.Count -eq 0) {
    Write-Host "Template spec bar drift check PASSED. No drift detected."
    exit 0
}

Write-Host ""
Write-Host "Template spec bar drift check FAILED. $($violations.Count) violation(s):"
Write-Host ""
foreach ($v in $violations) {
    Write-Host "  [$($v.rule)] $($v.template)"
    Write-Host "    $($v.detail)"
    Write-Host ""
}
exit 1
