# Execution Workflow

## For Release Plan Tasks

1. Read the task in `docs/release/AAA_RELEASE_EXECUTION_PLAN.md`.
2. Inspect the listed files before editing.
3. Implement the smallest change that satisfies the acceptance criteria.
4. Run the task's exact verification command.
5. If the command matches zero tests, fix the test names or the plan command so the check is meaningful.
6. Summarize changed files and verification.

## Creator-product preflight

Before changing code for the active creator-product plan, run
`./tools/ci/check_workspace_identity.ps1 -ExpectedRoot 'G:\URPG Maker-development'` on this workstation. The guard requires a clean worktree by default, validates the native URPG repository markers and normalized origin, and reports branch/upstream/integration-base/PR visibility. Use `-RequireClean:$false` only for read-only diagnosis of a deliberately dirty workspace; it is not a feature-work entry gate.

## For Non-Plan Tasks

1. Search first with `rg`.
2. Follow existing subsystem patterns.
3. Add or update tests when behavior changes.
4. Update canonical docs when status, release truth, or public behavior changes.
5. Run the narrowest quality gate from `docs/agent/QUALITY_GATES.md`.

## Review Stance

Code review should prioritize behavioral bugs, release-truth gaps, missing diagnostics, missing tests, and drift between docs and code. Style-only churn is secondary unless it protects an enforced invariant.
