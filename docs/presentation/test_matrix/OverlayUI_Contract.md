# Overlay/UI Presentation Contract

## Purpose
Defines the absolute visual priority for messages and user interface.

## Minimum Requirements
- **Absolute Readability Priority**: NO visual flair may compromise text legibility.
- **Stable Composition**: Must look identical regardless of the underlying presentation mode (Spatial vs 2D).
- **Contrast Guarantees**: Built-in contrast management independent of the world's post-FX stack.
- **Pass Isolation**: UI must be rendered in a final pass isolated from world blur/bloom.

## Fallback Policy
- None. UI correctness is non-negotiable.
