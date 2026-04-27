# URPG Privacy Policy

Last updated: 2026-04-26

Review status: implementation-accurate engineering policy; legal sufficiency is unverified until reviewed by a qualified legal reviewer.

URPG does not upload analytics by default. The editor starts with analytics consent set to `unknown`, keeps uploads disabled, and records a user decision only when the analytics settings control is changed.

## Analytics Data

When analytics are explicitly enabled, the local analytics dispatcher may collect:

- Event name
- Event category
- Deterministic timestamp tick
- Bounded string parameters supplied by the editor feature that emitted the event
- Session identifier used by the upload batch

The analytics validator rejects or reports empty event names, empty categories, disallowed categories, empty parameter keys or values, and excessive parameter counts. Analytics export redacts caller-specified PII keys through `AnalyticsPrivacyController::exportUserData`.

## Consent And Opt-Out

Analytics collection and upload are permitted only when consent is `granted`. The default state is `unknown`, which suppresses upload. Choosing the analytics opt-in control records granted consent. Turning it off records denied consent and disables analytics again.

The editor persists the consent state in:

```text
<project-root>/.urpg/settings/editor.json
```

Removing or corrupting that file causes the editor to recover to defaults, which means analytics consent returns to `unknown` and upload remains disabled.

## Uploads

The uploader is transport-agnostic and has no network upload handler configured by default in `urpg_editor`. The release editor configures a local JSONL export handler at `reports/analytics/editor_analytics.jsonl`; queued events are written there only when analytics opt-in is enabled and consent is granted.

URPG does not currently configure advertising identifiers, third-party analytics SDKs, or automatic crash-report uploads in the shipped editor/runtime entry points.

## Data Retention And Erasure

`AnalyticsPrivacyController` supports retention limits by maximum event age and maximum event count. It also supports local export and erasure workflows for queued analytics events. Erasure clears the supplied local event buffer; external storage or custom upload backends must implement their own matching deletion workflow before being enabled.

## Contact And Review

Until a public support channel is approved, privacy questions for release candidates must be routed through the project owner or release manager recorded in the release signoff workflow. A public release must replace this internal routing note with the approved support/contact address.
