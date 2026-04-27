# Execution Workflow

## For Release Plan Tasks

1. Read the task in `docs/release/AAA_RELEASE_EXECUTION_PLAN.md`.
2. Inspect the listed files before editing.
3. Implement the smallest change that satisfies the acceptance criteria.
4. Run the task's exact verification command.
5. If the command matches zero tests, fix the test names or the plan command so the check is meaningful.
6. Summarize changed files and verification.

## For Non-Plan Tasks

1. Search first with `rg`.
2. Follow existing subsystem patterns.
3. Add or update tests when behavior changes.
4. Update canonical docs when status, release truth, or public behavior changes.
5. Run the narrowest quality gate from `docs/agent/QUALITY_GATES.md`.

## Review Stance

Code review should prioritize behavioral bugs, release-truth gaps, missing diagnostics, missing tests, and drift between docs and code. Style-only churn is secondary unless it protects an enforced invariant.
