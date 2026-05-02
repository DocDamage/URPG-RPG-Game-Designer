$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "..\docs\check_template_claims.ps1")
