param(
    [string[]]$AiderArgs = @()
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
& (Join-Path $scriptDir "run-aider-provider.ps1") -EnvFile ".env.claude" -AiderArgs $AiderArgs
exit $LASTEXITCODE
