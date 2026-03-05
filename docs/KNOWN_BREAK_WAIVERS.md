# Known Break Waivers

Use `tools/ci/known_break_waivers.json` to track temporary CI waivers.

## Rules

- Every waiver must include:
  - `id`
  - `scope`
  - `issue_url`
  - `reason`
  - `expires_on` (ISO date)
- Expired waivers fail CI.
- Empty waiver list is valid and preferred.

## Example

```json
{
  "waivers": [
    {
      "id": "compat-weekly-001",
      "scope": "weekly",
      "issue_url": "https://github.com/DocDamage/URPG-RPG-Game-Designer/issues/123",
      "reason": "Intermittent plugin compat timeout on ubuntu-latest",
      "expires_on": "2026-04-01"
    }
  ]
}
```
