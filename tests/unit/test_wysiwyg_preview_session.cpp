#include "editor/events/event_command_graph_panel.h"
#include "editor/export/export_preview_panel.h"
#include "editor/message/dialogue_preview_panel.h"
#include "engine/core/wysiwyg/preview_session.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>

namespace {

urpg::localization::LocaleCatalog makePreviewSessionLocale() {
    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson({
        {"locale", "en-US"},
        {"keys", nlohmann::json::object()},
    });
    return catalog;
}

urpg::message::DialoguePreviewDocument makeRuntimeDialogueDocument() {
    urpg::message::DialoguePreviewDocument document;
    document.id = "dialogue_session_doc";
    document.pages.push_back({
        "intro",
        "Welcome.",
        "",
        urpg::message::variantFromCompatRoute("speaker", "Guide", 1),
        {{"accept", "Continue", "", "done", "open_gate", {{2, 1}}, true, ""}},
        0,
        true,
    });
    document.pages.push_back({
        "done",
        "Done.",
        "",
        urpg::message::variantFromCompatRoute("narration", "", 0),
        {},
        0,
        true,
    });
    document.portraits.registerBinding(1, {"portraits/guide.png", 0, false});
    return document;
}

urpg::events::EventCommandGraphDocument makeRuntimeEventGraphDocument() {
    urpg::events::EventCommandGraphDocument document;
    document.id = "event_session_graph";
    document.map_id = "town";
    document.event_id = "evt_gate";
    document.page_id = "main";
    document.entry_node_id = "set_switch";
    document.nodes = {
        {"set_switch", "Set switch", urpg::events::EventCommandKind::Switch, "gate_open", "true", 1, 0, 0, {}},
        {"set_variable", "Set variable", urpg::events::EventCommandKind::Variable, "gate_count", "", 2, 120, 0, {}},
    };
    document.edges = {{"edge_switch_to_variable", "set_switch", "set_variable", "sequence", "", true, "", 0}};
    return document;
}

urpg::exporting::ExportPreviewDocument makeExportDocument(const std::filesystem::path& output_dir) {
    urpg::exporting::ExportPreviewDocument document;
    document.id = "export_session_doc";
    document.mode = urpg::tools::ExportMode::DevBootstrap;
    document.output_dir = output_dir.string();
    document.expected_artifacts = {"data.pck", "game.exe"};
    return document;
}

bool hasEvidence(const urpg::wysiwyg::WysiwygPreviewSession& session, urpg::wysiwyg::PreviewEvidenceKind kind,
                 bool present) {
    return std::any_of(session.evidence.begin(), session.evidence.end(), [&](const auto& evidence) {
        return evidence.kind == kind && evidence.present == present;
    });
}

} // namespace

TEST_CASE("PreviewSession shared model records route source traces diagnostics evidence and confidence",
          "[PreviewSession][WYSIWYG]") {
    urpg::wysiwyg::WysiwygPreviewSession session;
    session.route_id = "route/dialogue";
    session.surface_id = "dialogue_preview";
    session.source_id = "dialogue_session_doc";
    session.mode = urpg::wysiwyg::PreviewMode::RuntimeBacked;
    session.runtime_trace_rows.push_back({"show_page", "intro", "runtime page displayed"});
    session.summary_rows.push_back({"speaker", "Guide", "speaker resolved"});
    session.evidence.push_back({urpg::wysiwyg::PreviewEvidenceKind::SavedData, true, "document saved"});
    session.recomputeConfidence();

    REQUIRE(session.route_id == "route/dialogue");
    REQUIRE(session.source_id == "dialogue_session_doc");
    REQUIRE(session.confidence.savedDataPresent);
    REQUIRE(session.confidence.runtimeBacked);
    REQUIRE(session.confidence.diagnosticsClean);
    REQUIRE_FALSE(session.confidence.exportAware);
    REQUIRE_FALSE(session.confidence.exactShipReady);
}

TEST_CASE("PreviewSession dialogue adapter exposes runtime-backed confidence and blocker diagnostics",
          "[PreviewSession][Message][WYSIWYG]") {
    urpg::editor::DialoguePreviewPanel panel;
    panel.loadDocument(makeRuntimeDialogueDocument(), makePreviewSessionLocale());
    panel.selectPage("intro");
    panel.selectChoice(0);
    panel.confirmSelectedChoice(true);
    panel.render();

    const auto& session = panel.previewSession();
    REQUIRE(session.route_id == "message/dialogue_preview");
    REQUIRE(session.surface_id == "dialogue_preview");
    REQUIRE(session.source_id == "dialogue_session_doc");
    REQUIRE(session.confidence.savedDataPresent);
    REQUIRE(session.confidence.runtimeBacked);
    REQUIRE(session.confidence.diagnosticsClean);
    REQUIRE_FALSE(session.confidence.exactShipReady);
    REQUIRE(hasEvidence(session, urpg::wysiwyg::PreviewEvidenceKind::SavedData, true));
    REQUIRE(hasEvidence(session, urpg::wysiwyg::PreviewEvidenceKind::RuntimeExecution, true));
    REQUIRE(session.runtime_trace_rows.size() >= 5);
    REQUIRE(session.summary_rows.size() >= 3);

    urpg::editor::DialoguePreviewPanel broken;
    urpg::message::DialoguePreviewDocument broken_document;
    broken_document.pages.push_back({"", "", "", urpg::message::variantFromCompatRoute("speaker", "", 0), {}, 0, true});
    broken.loadDocument(broken_document, makePreviewSessionLocale());
    broken.render();

    REQUIRE_FALSE(broken.previewSession().confidence.savedDataPresent);
    REQUIRE_FALSE(broken.previewSession().confidence.diagnosticsClean);
    REQUIRE_FALSE(broken.previewSession().confidence.runtimeBacked);
    REQUIRE_FALSE(broken.previewSession().diagnostics.empty());
}

TEST_CASE("PreviewSession event graph adapter exposes runtime execution and graph diagnostics",
          "[PreviewSession][Event Authoring][WYSIWYG]") {
    urpg::editor::EventCommandGraphPanel panel;
    panel.loadDocument(makeRuntimeEventGraphDocument());
    panel.render();

    const auto& session = panel.previewSession();
    REQUIRE(session.route_id == "events/event_command_graph");
    REQUIRE(session.source_id == "event_session_graph");
    REQUIRE(session.confidence.savedDataPresent);
    REQUIRE(session.confidence.runtimeBacked);
    REQUIRE(session.confidence.diagnosticsClean);
    REQUIRE(session.runtime_trace_rows.size() >= 3);
    REQUIRE(session.summary_rows.size() >= 2);

    auto broken_document = makeRuntimeEventGraphDocument();
    broken_document.id.clear();
    broken_document.entry_node_id = "missing";
    urpg::editor::EventCommandGraphPanel broken;
    broken.loadDocument(broken_document);
    broken.render();

    REQUIRE_FALSE(broken.previewSession().confidence.savedDataPresent);
    REQUIRE_FALSE(broken.previewSession().confidence.runtimeBacked);
    REQUIRE_FALSE(broken.previewSession().confidence.diagnosticsClean);
    REQUIRE_FALSE(broken.previewSession().diagnostics.empty());
}

TEST_CASE("PreviewSession export adapter is export-aware and blocks exact ship when artifacts are missing",
          "[PreviewSession][Export preview][WYSIWYG]") {
    const auto workspace = std::filesystem::temp_directory_path() / "urpg_preview_session_export";
    std::filesystem::remove_all(workspace);

    urpg::editor::ExportPreviewPanel panel;
    panel.loadDocument(makeExportDocument(workspace / "out"), workspace);
    panel.render();

    const auto& session = panel.previewSession();
    REQUIRE(session.route_id == "export/export_preview");
    REQUIRE(session.source_id == "export_session_doc");
    REQUIRE(session.confidence.savedDataPresent);
    REQUIRE(session.confidence.runtimeBacked);
    REQUIRE_FALSE(session.confidence.diagnosticsClean);
    REQUIRE(session.confidence.exportAware);
    REQUIRE_FALSE(session.confidence.exactShipReady);
    REQUIRE(hasEvidence(session, urpg::wysiwyg::PreviewEvidenceKind::ExportPackage, false));
    REQUIRE_FALSE(session.diagnostics.empty());

    std::filesystem::remove_all(workspace);
}
