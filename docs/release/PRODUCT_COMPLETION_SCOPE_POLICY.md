# Product Completion Scope and P0 Policy

The frozen P0/P1 release scope is machine-readable in
`content/readiness/product_completion_scope.json`. Work enters the release lane only
when it fixes a P0/P1, removes recorded debt, improves required evidence, or has a
separate recorded approval. Unapproved feature breadth is deferred by default.

Each scope entry names its owner paths or roles, priority, dependencies, acceptance
outcome, and deferral reason. Changing an entry requires updating the ledger and this
policy in the same review. A smaller claim may be approved only through an explicit
release decision; implementation must not silently redefine acceptance.

The accepted-defect ledger is
`content/readiness/product_completion_defects.json`. P0 means any credible data loss,
crash, unfinishable golden path, inaccessible required action, package corruption,
security/privacy leak, or false readiness claim. P0 defects cannot be waived for a
release candidate. They remain present until evidence closes them; removing a row is
not a valid disposition.

`tools/ci/check_product_completion_scope.ps1` validates both ledgers. Structural
checks may use `-AllowOpenP0` during implementation, but release-candidate creation
runs the strict form and fails while any accepted P0 is open.
