# AI Chatbot Capability Parity Design

## Goal

Make the in-app AI/chatbot capable of helping with every shipped editor workflow, while also making every dev-only, nested, and deferred editor surface discoverable without claiming unsupported mutation.

## Scope

Release top-level shipped workflows must be actionable or explicitly readonly through the chatbot. Dev-only, nested, and deferred panels must be searchable, explainable, and routable as knowledge entries, but their mutation stays disabled unless they later graduate into a reviewed release workflow.

## Architecture

The existing AI knowledge stack remains the source of truth:

- `AppCapabilityRegistry` describes what the app can do.
- `DocumentationKnowledgeIndex` exposes docs and editor panel knowledge.
- `AiToolRegistry` describes callable chatbot tools.
- `buildWysiwygChatbotCoverageReport()` proves parity between editor surfaces and chatbot coverage.

This pass adds access metadata instead of broad new mutation. Capabilities and tools gain explicit fields for editor exposure, AI access level, panel ownership, and promotion gates. The chatbot can then distinguish shipped, review-gated actions from discoverable support surfaces.

## Access Levels

- `release_actionable`: a shipped top-level workflow with at least one reviewed chatbot tool. Mutating tools must require approval and remain revertable through existing JSON Patch history.
- `release_readonly`: a shipped top-level workflow the chatbot can inspect, explain, validate, preview, or route to, but cannot mutate yet.
- `dev_discoverable`: a non-release workflow that is searchable and explainable, with optional route/help support only.
- `unsupported`: a known workflow without safe chatbot coverage. It must provide a promotion gate before it can become actionable.

## Behavior

Every `EditorPanelRegistryEntry` is indexed into chatbot knowledge, not only release top-level panels. Release top-level panels are expected to have either direct actionable capability coverage or a readonly capability/tool fallback. Non-release panels are expected to be discoverable as knowledge entries and to carry an access level that prevents accidental mutation.

The chatbot gains non-mutating planning tools for describing workflows, listing panel actions, and routing users to an editor panel. These tools can be used for dev-only/deferred panels because they do not write project data.

Existing mutating tools remain review-gated. The coverage report must flag any mutating tool that does not require approval.

## Data Flow

1. `buildDefaultAiKnowledgeSnapshot()` builds capabilities, project knowledge, docs knowledge, and tools.
2. `DocumentationKnowledgeIndex::buildDefault()` indexes all editor panels with exposure/access metadata.
3. `AppCapabilityRegistry::buildDefault()` registers shipped capabilities and auto-registers readonly/discoverable panel capabilities for registry entries not already covered.
4. `AiTaskPlanner` maps requests to actionable tools when safe, otherwise falls back to readonly describe/route tools.
5. `AiToolRegistry::applyApprovedPlan()` records non-mutating panel tool previews without changing project data.
6. Coverage tests prove no editor panel is invisible to chatbot knowledge and no mutating tool bypasses approval.

## Error Handling

Unsupported or deferred workflows must not throw or silently mutate. They should return diagnostics or readonly previews that explain the current access level and promotion gate. Unknown panel ids are treated as validation errors.

## Testing

Focused tests cover:

- every editor panel is searchable by chatbot knowledge;
- release top-level panels have actionable or readonly chatbot coverage;
- dev/deferred/nested panels are discoverable but not mutating;
- mutating tools require approval;
- panel describe/route/list-action tools produce previews without project patches;
- chatbot command flow can plan and apply a non-mutating panel-routing request.

