# Unified LLM Workflow Launcher

This folder provides a one-shot setup command for any project:

- Load local env files (`.env`, `.contextlattice/orchestrator.env`)
- Ensure project tool folders exist:
  - `tools/codemunch`
  - `tools/contextlattice`
  - `tools/memorybridge`
- Install required runtime dependencies when missing:
  - `codemunch-pro`
  - `chromadb`
- Normalize provider credentials for tool compatibility:
  - OpenAI (`OPENAI_API_KEY`, optional `OPENAI_BASE_URL`)
  - Kimi (`KIMI_API_KEY`, optional `KIMI_BASE_URL`)
  - Gemini (`GEMINI_API_KEY`, optional `GEMINI_BASE_URL`)
  - GLM (`GLM_API_KEY`, optional `GLM_BASE_URL`)
- Bootstrap and verify:
  - CodeMunch project files
  - ContextLattice project files + connectivity check
  - MemPalace bridge files + dry-run sync

## Install global command

From this repository:

```powershell
.\tools\workflow\install-global-llm-workflow.ps1
```

Then open a new PowerShell session and run in any project root:

```powershell
llm-workflow-up
```

Alias:

```powershell
llmup
```

Strict full check command:

```powershell
llm-workflow-check
```

Alias:

```powershell
llmcheck
```

Diagnostics command:

```powershell
llm-workflow-doctor
```

Alias:

```powershell
llmdoctor
```

`llm-workflow-check` runs a strict end-to-end validation:

- provider normalization
- codemunch index run (`-Embed`)
- ContextLattice smoke write+search (warns if search is delayed by async indexing)
- MemPalace bridge dry-run (`-Strict`)

## Useful flags

```powershell
llm-workflow-up -SkipDependencyInstall
llm-workflow-up -Provider glm
llm-workflow-up -Provider gemini
llm-workflow-up -SkipContextVerify
llm-workflow-up -SkipBridgeDryRun
llm-workflow-up -SmokeTestContext
llm-workflow-up -SmokeTestContext -RequireSearchHit
llm-workflow-up -DeepCheck
llm-workflow-up -FailIfNoProviderKey
llm-workflow-check -Provider kimi
llm-workflow-check -Provider glm -ContextSearchAttempts 120 -ContextSearchDelaySec 2
llm-workflow-up -DeepCheck -RequireSearchHit
llm-workflow-doctor -Provider auto -CheckContext -Strict
```

## Sprint handoff packs

For repo-local sprint execution where one LLM session may hand off to another, keep the active sprint plan and task board in-repo and point the next session at them first.

Stable entrypoint for any model:

- `docs/superpowers/plans/ACTIVE_SPRINT.md`
- `tools/workflow/SPRINT_AGENT_HANDOFF_TEMPLATE.md`

Create a new sprint pack:

```powershell
.\tools\workflow\new-sprint-pack.ps1 `
  -SprintId S06 `
  -Slug "sprint-06-example" `
  -Goal "Execute the next bounded sprint end-to-end with resumable LLM handoff." `
  -Theme "explicit sprint ownership and handoff hygiene" `
  -PrimaryOutcomes "land bounded ticket work", "keep docs and readiness aligned" `
  -NonGoals "no broad roadmap expansion", "no unlabeled status promotion" `
  -Activate
```

That command creates:

- a dated execution pack in `docs/superpowers/plans/`
- a paired task board in `docs/superpowers/plans/`
- an updated `ACTIVE_SPRINT.md` pointer so the next LLM session knows exactly where to start

Suggested pattern:

1. Generate or update the sprint pack.
2. Open `ACTIVE_SPRINT.md`.
3. Mark one ticket `IN PROGRESS` in the task board.
4. Run the ticket-local verification command before editing.
5. Update docs and readiness records in the same change as code.
6. Leave a resume note in the task board before ending the session.
