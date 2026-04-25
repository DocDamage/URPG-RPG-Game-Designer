# Certification Schema Notes

Status Date: 2026-04-25

FFS-17 introduces lightweight JSON payloads rather than hard runtime save schemas:

- `template_certification`: certification report output for template loop checks.
- `project_completeness_score`: project audit advisory completeness score.
- `feature_governance_manifest`: local feature governance manifest shape.

All three are additive, advisory, and not a release gate. Migration impact is none.
