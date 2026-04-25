# URPG Mod SDK

URPG mods are local project extensions. The SDK sample in `content/fixtures/mod_sdk_sample/`
shows the minimum manifest shape, the permission vocabulary accepted for bundled samples, and
the diagnostics contract expected by validation tools.

The in-tree sample deliberately uses a deny-by-default permission model:

- `read_project_data` allows read-only inspection of project database content.
- `register_commands` allows editor/runtime command registration through approved APIs.
- `emit_diagnostics` allows structured diagnostics to be written for review.

Network, native interop, and arbitrary filesystem write permissions are not part of the bundled
sample contract. Integrations that need those capabilities must define an out-of-tree policy and
cannot rely on the sample validator to bless them.
