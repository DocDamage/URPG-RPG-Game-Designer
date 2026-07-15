#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace urpg::editor {
enum class EditorAssetProvenanceState { RawExternal, Promoted, Attached };
struct EditorAssetDragPayload { std::string assetId; std::string projectPath; std::string mediaKind; std::string attachmentRevision; uint32_t width = 0; uint32_t height = 0; EditorAssetProvenanceState provenance = EditorAssetProvenanceState::RawExternal; };
struct EditorAssetDropDecision { bool accepted = false; std::string code; std::string message; std::string remediation; };
const char* toString(EditorAssetProvenanceState state);
EditorAssetProvenanceState editorAssetProvenanceStateFromString(const std::string& state);
std::vector<std::uint8_t> serializeEditorAssetDragPayload(const EditorAssetDragPayload& payload);
EditorAssetDropDecision deserializeEditorAssetDragPayload(const std::vector<std::uint8_t>& bytes, EditorAssetDragPayload* payload);
EditorAssetDropDecision assessEditorAssetDrop(const EditorAssetDragPayload& payload, bool durableProjectDocument);
EditorAssetDropDecision validateEditorAssetAttachmentRevision(const EditorAssetDragPayload& payload,
                                                              const std::filesystem::path& projectRoot);
} // namespace urpg::editor
