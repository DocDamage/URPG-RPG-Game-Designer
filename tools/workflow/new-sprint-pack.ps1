param(
  [Parameter(Mandatory = $true)]
  [string]$SprintId,

  [Parameter(Mandatory = $true)]
  [string]$Slug,

  [Parameter(Mandatory = $true)]
  [string]$Goal,

  [Parameter(Mandatory = $true)]
  [string]$Theme,

  [string[]]$PrimaryOutcomes = @(" - <fill me>"),
  [string[]]$NonGoals = @(" - <fill me>"),
  [string]$DateStamp = (Get-Date -Format "yyyy-MM-dd"),
  [string]$PlansRoot = "docs/superpowers/plans",
  [string]$TemplateRoot = "tools/workflow",
  [string]$ActiveSprintFile = "docs/superpowers/plans/ACTIVE_SPRINT.md",
  [switch]$Activate
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-RepoPath {
  param(
    [Parameter(Mandatory = $true)]
    [string]$PathValue
  )

  if ([System.IO.Path]::IsPathRooted($PathValue)) {
    return [System.IO.Path]::GetFullPath($PathValue)
  }

  return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $PathValue))
}

function Ensure-ParentDirectory {
  param(
    [Parameter(Mandatory = $true)]
    [string]$FilePath
  )

  $parent = Split-Path -Parent $FilePath
  if (-not (Test-Path -LiteralPath $parent)) {
    New-Item -ItemType Directory -Path $parent -Force | Out-Null
  }
}

function Replace-TemplateTokens {
  param(
    [Parameter(Mandatory = $true)]
    [string]$TemplateContent,

    [Parameter(Mandatory = $true)]
    [hashtable]$Tokens
  )

  $rendered = $TemplateContent
  foreach ($key in $Tokens.Keys) {
    $rendered = $rendered.Replace(("{{" + $key + "}}"), [string]$Tokens[$key])
  }
  return $rendered
}

$plansRootPath = Resolve-RepoPath -PathValue $PlansRoot
$templateRootPath = Resolve-RepoPath -PathValue $TemplateRoot
$activeSprintPath = Resolve-RepoPath -PathValue $ActiveSprintFile

$executionTemplatePath = Join-Path $templateRootPath "SPRINT_EXECUTION_PACK_TEMPLATE.md"
$taskBoardTemplatePath = Join-Path $templateRootPath "SPRINT_TASK_BOARD_TEMPLATE.md"

if (-not (Test-Path -LiteralPath $executionTemplatePath)) {
  throw "Execution pack template not found: $executionTemplatePath"
}

if (-not (Test-Path -LiteralPath $taskBoardTemplatePath)) {
  throw "Task board template not found: $taskBoardTemplatePath"
}

$executionPackFileName = "{0}-{1}-execution-pack.md" -f $DateStamp, $Slug
$taskBoardFileName = "{0}-{1}-task-board.md" -f $DateStamp, $SprintId.ToLowerInvariant()

$executionPackPath = Join-Path $plansRootPath $executionPackFileName
$taskBoardPath = Join-Path $plansRootPath $taskBoardFileName

if ((Test-Path -LiteralPath $executionPackPath) -or (Test-Path -LiteralPath $taskBoardPath)) {
  throw "Refusing to overwrite an existing sprint pack. Remove or rename the target files first."
}

$primaryOutcomesBlock = ($PrimaryOutcomes | ForEach-Object {
  if ($_ -match '^\s*-\s') { $_ } else { "- $_" }
}) -join [Environment]::NewLine

$nonGoalsBlock = ($NonGoals | ForEach-Object {
  if ($_ -match '^\s*-\s') { $_ } else { "- $_" }
}) -join [Environment]::NewLine

$templateTokens = @{
  "SPRINT_ID" = $SprintId
  "GOAL" = $Goal
  "THEME" = $Theme
  "PRIMARY_OUTCOMES" = $primaryOutcomesBlock
  "NON_GOALS" = $nonGoalsBlock
}

$executionTemplate = Get-Content -LiteralPath $executionTemplatePath -Raw
$taskBoardTemplate = Get-Content -LiteralPath $taskBoardTemplatePath -Raw

$renderedExecutionPack = Replace-TemplateTokens -TemplateContent $executionTemplate -Tokens $templateTokens
$renderedTaskBoard = Replace-TemplateTokens -TemplateContent $taskBoardTemplate -Tokens $templateTokens

Ensure-ParentDirectory -FilePath $executionPackPath
Ensure-ParentDirectory -FilePath $taskBoardPath

[System.IO.File]::WriteAllText($executionPackPath, $renderedExecutionPack)
[System.IO.File]::WriteAllText($taskBoardPath, $renderedTaskBoard)

if ($Activate) {
  $resolvedExecutionPackPath = (Resolve-Path -LiteralPath $executionPackPath).Path
  $resolvedTaskBoardPath = (Resolve-Path -LiteralPath $taskBoardPath).Path

  $activeSprintContent = @"
# Active Sprint Pointer

This file tells any LLM session which sprint artifacts to open first.

## Active Sprint

- Sprint ID: $SprintId
- Execution pack: $resolvedExecutionPackPath
- Task board: $resolvedTaskBoardPath

## Start Order

1. Open the execution pack.
2. Open the task board.
3. Read the `Resume From Here` section before editing code.
4. Mark exactly one ticket `IN PROGRESS`.
5. Leave the task board current before ending the session.
"@

  Ensure-ParentDirectory -FilePath $activeSprintPath
  [System.IO.File]::WriteAllText($activeSprintPath, $activeSprintContent)
}

Write-Output ("[new-sprint-pack] execution_pack={0}" -f $executionPackPath)
Write-Output ("[new-sprint-pack] task_board={0}" -f $taskBoardPath)
if ($Activate) {
  Write-Output ("[new-sprint-pack] active_sprint={0}" -f $activeSprintPath)
}
