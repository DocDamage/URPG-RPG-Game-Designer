# Feature Governance

Status Date: 2026-04-25

Feature governance manifests are stable pointers to owner, docs, schema or canonical data, and
tests. The script checks that those pointers resolve locally and that generated or missing files do
not accidentally become accepted evidence.

This is advisory and not a release gate unless a future sprint explicitly wires a specific manifest
into release readiness. Unsupported scope includes live-service checks and external marketplace
validation. Disabled optional features remain opt-out evidence and should not be promoted to hard
requirements.
