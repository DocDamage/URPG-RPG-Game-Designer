# PFU-06 Work Packet: project localization-reference audit

**Status:** Implemented; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Provide a native, read-only project audit of stable localization keys used by
saved Dialogue Graph and Quest Objective Graph documents against the union of
valid project locale bundles.

## Contract

1. Valid locale-bundle keys under `content/localization` form the available
   project-key set.
2. Saved `content/dialogues/*.json` node/caption keys and
   `content/quests/*.json` node keys form typed references with document path,
   owner kind, and local node ID. Dialogue nodes with voice/caption metadata
   also form read-only media rows.
3. The audit reports invalid documents/bundles as diagnostics, missing
   referenced keys, per-locale gaps for referenced keys, locale bundles without
   a declared font profile, and unused-key candidates in deterministic order.
4. The Map creator surface exposes the audit on demand. It does not modify any
   bundle or document. An unused-key candidate is never deletion authority
   because other localization owners remain outside this bounded scan.
5. The same creator surface provides a deterministic pseudo-localization
   preview that accents ASCII vowels, expands alphabetic text, and brackets the
   result without persisting or rewriting the source text.
6. For each Dialogue media row, the audit checks a matching project-contained,
   runtime-ready attached audio manifest and caption-key availability. It
   reports missing voice assets, voice-without-caption, caption-without-voice,
   and missing caption-key issues without modifying either owner.

## Limits

Menu, database, event, extension, message, template, and runtime references
remain later audit increments. Translation import/export, full persisted
pseudo-localization workflows, RTL/IME/plural/layout validation, font/glyph
checks, and manual assistive review remain PFU-06 work. The bounded preview is
not an RTL/IME, plural, glyph-coverage, or end-to-end layout qualification.
Voice takes/locales/rights, muted alternatives, automatic caption generation,
device playback, and listening review remain F24 work.

Verification is deferred by user instruction.
