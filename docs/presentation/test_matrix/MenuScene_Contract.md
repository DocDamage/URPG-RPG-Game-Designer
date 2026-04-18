# MenuScene Presentation Contract

## Purpose
Defines constraints for UI-heavy background presentation.

## Minimum Requirements
- **Layered Backgrounds**: Support for Parallax or 3D-staged backgrounds behind UI.
- **Depth Control**: Explicit visual depth that doesn't compete with Top-most UI components.
- **Safe Post-FX**: Global effects (blur, color grade) that do not degrade text legibility.
- **Cost Ceiling**: Budget is STRICTLY limited (e.g., < 2ms) to ensure menu responsiveness.

## Fallback Policy
- Static images or very low-cost scrolling 2D textures.
- Disable all background post-FX if performance targets are breached.
