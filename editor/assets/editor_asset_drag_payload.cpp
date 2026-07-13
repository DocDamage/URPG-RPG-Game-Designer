#include "editor/assets/editor_asset_drag_payload.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {
const char* toString(EditorAssetProvenanceState state) { return state == EditorAssetProvenanceState::Promoted ? "promoted" : state == EditorAssetProvenanceState::Attached ? "attached" : "raw_external"; }
EditorAssetProvenanceState editorAssetProvenanceStateFromString(const std::string& state) { return state == "promoted" ? EditorAssetProvenanceState::Promoted : state == "attached" ? EditorAssetProvenanceState::Attached : EditorAssetProvenanceState::RawExternal; }
std::vector<std::uint8_t> serializeEditorAssetDragPayload(const EditorAssetDragPayload& payload) {
    const auto text = nlohmann::json{{"schema", "urpg.editor_asset_drag.v1"}, {"asset_id", payload.assetId}, {"project_path", payload.projectPath}, {"media_kind", payload.mediaKind}, {"width", payload.width}, {"height", payload.height}, {"provenance", toString(payload.provenance)}}.dump();
    return {text.begin(), text.end()};
}
EditorAssetDropDecision deserializeEditorAssetDragPayload(const std::vector<std::uint8_t>& bytes, EditorAssetDragPayload* payload) {
    const auto value = nlohmann::json::parse(bytes.begin(), bytes.end(), nullptr, false);
    if (value.is_discarded() || !value.is_object() || value.value("schema", "") != "urpg.editor_asset_drag.v1") return {false, "asset_drag_payload_invalid", "The dragged asset payload is incompatible.", "Drag the asset again."};
    EditorAssetDragPayload parsed;
    parsed.assetId = value.value("asset_id", ""); parsed.projectPath = value.value("project_path", ""); parsed.mediaKind = value.value("media_kind", ""); parsed.width = value.value("width", 0U); parsed.height = value.value("height", 0U); parsed.provenance = editorAssetProvenanceStateFromString(value.value("provenance", "raw_external"));
    if (parsed.assetId.empty() || parsed.mediaKind.empty()) return {false, "asset_drag_payload_incomplete", "The dragged asset has no stable identity or media type.", "Select the asset from the Assets workspace again."};
    if (payload) *payload = std::move(parsed);
    return {true, "asset_drag_payload_valid", "Asset payload is valid.", ""};
}
EditorAssetDropDecision assessEditorAssetDrop(const EditorAssetDragPayload& payload, bool durableProjectDocument) {
    if (payload.assetId.empty() || payload.mediaKind.empty()) return {false, "asset_drop_invalid", "Asset identity and media type are required.", "Select an asset from Assets."};
    if (durableProjectDocument && payload.provenance == EditorAssetProvenanceState::RawExternal) return {false, "asset_drop_requires_attachment", "Raw external assets cannot be written into project documents.", "Use Attach To Project to review, govern, and attach this asset first."};
    if (durableProjectDocument && payload.projectPath.empty() && payload.provenance == EditorAssetProvenanceState::Promoted) return {false, "asset_drop_requires_project_path", "Promoted assets need a project attachment before this drop.", "Use Attach To Project before assigning this asset."};
    return {true, "asset_drop_accepted", "Asset drop accepted.", ""};
}
} // namespace urpg::editor
