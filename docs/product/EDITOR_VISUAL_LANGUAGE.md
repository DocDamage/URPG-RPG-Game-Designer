# Editor Visual Language

The native editor uses a compact dark workspace designed for long authoring sessions. `editor/ui/editor_theme.*` is the single token source for surface, selection, focus, severity, spacing, control height, icon size, and rounding; `editor/ui/editor_widgets.*` provides the shared status, empty-state, diagnostic, and labeled-command primitives.

Important commands always retain a text label. Icons can clarify a command but never replace its accessible name. Disabled controls state what must change to enable them, and severity messages pair a stable diagnostic code with a creator-readable next action.

The interface-scale preference is stored in local editor accessibility settings and applies the same tokens at 100%, 125%, 150%, and 200% (with bounded support from 75% to 200%). This establishes the implementation baseline; graphical review remains a release qualification activity and must record the tested resolutions, DPI settings, screenshots, and any remaining issues in the release evidence for the target build.
