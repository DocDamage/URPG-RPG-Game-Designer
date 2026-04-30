# URPG Documentation Index

This folder is organized by document purpose. Use the active sprint pointer for current execution work, and use archive folders only for historical traceability.

## Current Execution

- [Active Sprint Pointer](./superpowers/plans/ACTIVE_SPRINT.md)
- [Release Surface Audit Execution Plan](./superpowers/plans/2026-04-30-release-surface-audit-execution-plan.md)
- [AAA Release Readiness Execution Plan](./superpowers/plans/2026-04-26-aaa-release-readiness-execution-plan.md)
- [AAA Release Readiness Report](./release/AAA_RELEASE_READINESS_REPORT.md)
- [Editor Control Inventory](./release/EDITOR_CONTROL_INVENTORY.md)

## Current Reference Folders

- `release/` - release readiness, packaging, waivers, and signoff workflow.
- `status/` - current program status snapshots.
- `governance/` - readiness matrices, schema changelog, truth-alignment rules, and labeling rules.
- `specs/` - native subsystem specifications and design contracts.
- `signoff/` - closure and signoff evidence packets.
- `audits/` - current audit snapshots and technical-debt audit records.
- `asset_intake/` - asset source, promotion, gap, and intake documentation.
- `external-intake/` - external repository intake governance and license tracking.
- `integrations/` - integration notes and external tooling references.
- `presentation/` - presentation subsystem validation, contracts, risks, and performance guidance.
- `templates/` - game-template specifications.
- `adr/` - architecture decision records.

## Root Compatibility Docs

Several `docs/*.md` files intentionally remain at the docs root because CI gates and agent workflows still read those exact paths. When a matching categorized copy exists under `release/`, `status/`, `governance/`, `specs/`, `signoff/`, or `audits/`, treat the categorized folder as the human browsing location and keep the root copy aligned until the gates are migrated.

## Archive

- `archive/planning/` - superseded plans, task boards, roadmap documents, and historical planning references.
- `archive/artifacts/` - historical status or kickoff artifacts.
- `archive/blueprints/` - superseded blueprint material.
