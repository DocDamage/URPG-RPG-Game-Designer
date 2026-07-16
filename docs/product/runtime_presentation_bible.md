# URPG Runtime Presentation Bible (Candidate 1.0)

Status: implementation candidate; design approval and in-engine visual review are pending.

URPG’s presentation language is a playful field workshop: warm, tactile surfaces; inked field-guide shapes; sky-blue action signals; gold focus; compact star and paper motifs; and direct, player-led framing. It must remain original to URPG and must not borrow another platform holder’s characters, trade dress, sounds, controller shapes, or branded visual language.

The executable authority is `engine/core/presentation/runtime_presentation_bible.cpp`, backed by `engine/core/ui/urpg_design_tokens.cpp`. This document explains the intent; runtime code supplies exact values and validation.

## Visual system

- Palette roles are semantic: surface, raised surface, text, muted text, accent, danger, warning, success, and focus. Light, dark, and high-contrast variants preserve text and focus contrast.
- Typography uses five roles: caption, body, label, heading, and title. UI scale changes the complete type and spacing system together.
- Spacing follows a six-step 2/4/8/12/18/24 rhythm. Controls, cards, and popovers use bounded corner and elevation roles.
- Icons use only `urpg.icon.*` identities. Meaning never depends on an icon or color alone.

## Motion, particles, and camera

- Feedback, transition, and emphasis motion is short and interruptible. Reduced motion sets those durations to zero.
- Particles use paper spark, ink arc, star notch, and soft dust motifs. At most 24 emitters may coexist and particles live no longer than 1.2 seconds. Reduced motion uses at most 15% density.
- Camera framing follows the player. Emphasis is bounded to 6 pixels of shake, 65 ms hit stop, and 160 ms transitions. Reduced motion removes camera shake.

## Sound language

Focus, confirm, cancel, error, reward, and quest signals use original `urpg.sound.*` motifs. Sound reinforces state but never carries the only signal. Every showcase component remains understandable with audio disabled.

## Feedback hierarchy

Critical feedback can interrupt lower tiers; primary feedback may replace secondary or ambient feedback; secondary feedback never blocks control; ambient feedback never interrupts. Durations are capped at 1.5 seconds and every critical, primary, and secondary signal has a non-color cue.

## Accessibility variants

The required variants are default, large text, high contrast, and reduced motion. Each keeps audio optional and preserves non-color cues. The deterministic component showcase covers primary button, dialogue choice, interaction prompt, quest notification, error recovery card, progress indicator, and combat value states under every variant.

## Review questions

For each component and interaction, reviewers must be able to identify what changed, why it changed, what is possible next, whether input was acknowledged immediately, whether the dominant signal is clear, whether it works without audio/color/motion, whether it can be interrupted or backed out of, and whether it unmistakably belongs to URPG.
