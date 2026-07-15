# PFU-04 Dialogue Choice Localization Reference

## Scope

Add an optional, stable project localization key to native Dialogue Graph choice
labels. The existing graph remains the authoritative owner; this does not change
runtime dialogue traversal or replace legacy freeform labels.

## Contract

- The Dialogue Authoring controls let a creator type or select a project locale
  key for each choice alongside its visible label and target, and the
  non-persistent interactive preview shows that reference beside its label.
- The key is saved with the choice, survives the existing draft persistence and
  local undo/redo route, and rejects malformed JSON field types on load.
- Existing graphs without a choice key remain loadable and valid.
- Graph diagnostics and the read-only project localization audit identify a
  selected choice key that is absent from the active locale catalog.

## Limits

This does not localize a label automatically, change preview/runtime execution,
rewrite locale bundles, or qualify RTL/IME/glyph/layout behavior. Builds and
tests are deferred under the user instruction for this phase.
