# BND-009 Modern UI Starter Payload License Evidence

Status: License evidence provided
Related bundle manifest: `imports/manifests/asset_bundles/BND-009-modern-ui-starter-payload.json`
Related issue: #27
Related quarantine PR: #22
Recorded date: 2026-05-23
Evidence source: User-provided license text in ChatGPT conversation

## Applies To

This evidence applies to the candidate starter payload assets listed in `BND-009-modern-ui-starter-payload.json`, including the currently listed `cutesckr` tile candidates and `human_rpg_portraits` candidates.

The payload remains deferred until the exact 14 PNG files are copied and verified as real binary blobs.

## License Summary

The provided license grants permission to:

- use the assets in any project, including games, apps, and films
- use the assets for commercial or non-commercial purposes
- modify, remix, and redistribute the assets
- use the assets without attribution

The provided license identifies the assets as a Public Domain Dedication under CC0 1.0. If the public-domain dedication is not valid in a jurisdiction, the fallback grant is royalty-free, non-exclusive, irrevocable, worldwide, and permits use for any purpose, including commercial use.

## Warranty and Rights Caveats

The assets are provided as-is without warranty. The license text does not cover trademarks, patents, publicity rights, or privacy rights. Those rights must be cleared separately if a specific asset contains recognizable people, brands, or protected marks.

## Full Provided License Text

```text
You are free to:

Use these assets in any project (games, apps, films, etc.)
Use them for commercial or non-commercial purposes
Modify, remix, and redistribute them
Use them without attribution (credit is optional, not required)
Public Domain Dedication (CC0 1.0)

To the fullest extent permitted by law, the creator(s) of these assets have waived all copyright and related rights to these assets worldwide.

You may use the assets for any purpose whatsoever, without permission, restrictions, or attribution.

Fallback License

If any part of the public domain dedication is not legally valid in your jurisdiction, you are granted a:

Royalty-free
Non-exclusive
Irrevocable
Worldwide license

to use the assets for any purpose, including commercial use.

No Warranty

These assets are provided “as-is”, without warranty of any kind, express or implied, including but not limited to:

Merchantability
Fitness for a particular purpose
Non-infringement

The creator(s) are not liable for any damages or legal claims arising from their use.

Other Rights

This license does not cover:

Trademarks
Patents
Publicity or privacy rights (e.g., recognizable people, brands)

You are responsible for clearing those rights if needed.
```

## Promotion Rules

Before `BND-009` becomes release eligible, a payload PR must still:

- copy only the exact candidate files listed in the bundle manifest
- verify that each file is a real PNG binary blob
- update `.gitattributes` only for the approved payload paths
- run the manifest validator
- run release-required asset and package smoke checks
- keep release-required status false unless release ownership explicitly promotes the payload
