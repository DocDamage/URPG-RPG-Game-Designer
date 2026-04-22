param(
    [string[]]$AiderArgs = @()
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
& (Join-Path $scriptDir "run-aider-provider.ps1") -EnvFile ".env.moonshot" -AiderArgs $AiderArgs
exit $LASTEXITCODE
