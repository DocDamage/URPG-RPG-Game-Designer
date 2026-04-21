# Sprint Agent Handoff Template

Use this template to start or resume a sprint-focused LLM session in this repo.

Start from the active sprint pointer:

- `docs/superpowers/plans/ACTIVE_SPRINT.md`

## Fresh Session Prompt

```text
You are continuing the active sprint in the URPG repository.

Follow these files first:
- docs/superpowers/plans/ACTIVE_SPRINT.md
- docs/PROGRAM_COMPLETION_STATUS.md
- docs/TECHNICAL_DEBT_REMEDIATION_PLAN.md
- content/readiness/readiness_status.json

Rules:
- Work exactly one ticket at a time from the task board.
- Update docs in the same change as implementation.
- Do not silently drop unsupported migration behavior; emit diagnostics or explicit waivers.
- Do not relabel anything READY without matching evidence and CI/readiness updates.
- Before ending the session, update the task board resume note with:
  - current ticket status
  - last green command
  - first failing command
  - touched files
  - any blockers

Start by:
1. reading ACTIVE_SPRINT.md, then the execution pack and task board it points to
2. marking the active ticket IN PROGRESS
3. running the listed verification command for that ticket
4. implementing the ticket end-to-end if feasible in this session
```

## Mid-Sprint Resume Prompt

```text
Resume the active sprint from docs/superpowers/plans/ACTIVE_SPRINT.md.

Use the execution pack named there as the authority for ordering and acceptance criteria.

Before coding:
- read the Resume From Here section
- verify the last recorded failing command
- inspect touched files before making new edits

During work:
- keep the task board current
- keep docs and readiness records aligned with code
- leave a clean handoff note before you stop
```

## End-of-Session Handoff Note Template

```text
Current ticket: S01-T0X - <title>
Status: TODO / IN PROGRESS / BLOCKED / DONE
Last green command:
<command>

First failing command:
<command>

Touched files:
- <path>
- <path>

Docs updated:
- <path>
- <path>

Blockers:
- <short note or "none">

Next recommended action:
- <single next step>
```
