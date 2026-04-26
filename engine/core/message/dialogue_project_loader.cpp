#include "engine/core/message/dialogue_project_loader.h"

#include "engine/core/message/dialogue_database.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace urpg::message {

namespace {

MessagePresentationMode modeFromString(const std::string& value) {
    if (value == "narration") {
        return MessagePresentationMode::Narration;
    }
    if (value == "system") {
        return MessagePresentationMode::System;
    }
    return MessagePresentationMode::Speaker;
}

MessageTone toneFromString(const std::string& value) {
    if (value == "neutral") {
        return MessageTone::Neutral;
    }
    if (value == "system") {
        return MessageTone::System;
    }
    return MessageTone::Portrait;
}

std::vector<DialogueNode> nodesFromSequencePages(const nlohmann::json& pages) {
    std::vector<DialogueNode> nodes;
    if (!pages.is_array()) {
        return nodes;
    }

    nodes.reserve(pages.size());
    for (size_t index = 0; index < pages.size(); ++index) {
        const auto& page = pages[index];
        if (!page.is_object()) {
            continue;
        }

        DialogueNode node;
        node.id = page.value("id", "");
        node.body = page.value("body", "");
        node.command = page.value("command", "");
        node.variant.speaker = page.value("speaker", "");
        node.variant.face_actor_id = page.value("face_actor_id", 0);
        node.variant.mode = modeFromString(page.value("presentation_mode", "speaker"));
        node.variant.tone = toneFromString(page.value("tone", "portrait"));
        node.variant.route_token = page.value("source_route", "");

        if (const auto choices_it = page.find("choices"); choices_it != page.end() && choices_it->is_array()) {
            for (const auto& choice_json : *choices_it) {
                if (!choice_json.is_object()) {
                    continue;
                }
                node.choices.push_back({
                    .id = choice_json.value("id", ""),
                    .label = choice_json.value("label", ""),
                    .enabled = choice_json.value("enabled", true),
                    .disabled_reason = choice_json.value("disabled_reason", ""),
                });
            }
        }

        if (index + 1 < pages.size()) {
            const auto& next = pages[index + 1];
            if (next.is_object()) {
                node.next_node_id = next.value("id", "");
            }
        }

        if (!node.id.empty()) {
            nodes.push_back(std::move(node));
        }
    }

    return nodes;
}

std::vector<std::filesystem::path> candidateDialoguePaths(const std::filesystem::path& project_root) {
    return {
        project_root / "content" / "dialogue_sequences.json",
        project_root / "content" / "dialogue" / "dialogue_sequences.json",
        project_root / "content" / "data" / "dialogue_sequences.json",
    };
}

void addDiagnostic(DialogueProjectLoadResult& result, DialogueLoadSeverity severity, std::string code,
                   std::filesystem::path path, std::string message) {
    result.diagnostics.push_back({
        severity,
        std::move(code),
        std::move(path),
        std::move(message),
    });
}

} // namespace

DialogueProjectLoadResult DialogueProjectLoader::loadProject(DialogueRegistry& registry,
                                                             const std::filesystem::path& project_root) {
    DialogueProjectLoadResult result;
    const auto candidates = candidateDialoguePaths(project_root);

    std::filesystem::path selected_path;
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            selected_path = candidate;
            break;
        }
    }

    if (selected_path.empty()) {
        addDiagnostic(result, DialogueLoadSeverity::Warning, "dialogue_content_missing", project_root,
                      "No project dialogue_sequences.json file was found.");
        return result;
    }

    result.source_path = selected_path;
    try {
        std::ifstream input(selected_path);
        if (!input) {
            addDiagnostic(result, DialogueLoadSeverity::Error, "dialogue_content_open_failed", selected_path,
                          "Project dialogue file could not be opened.");
            return result;
        }

        nlohmann::json document;
        input >> document;
        result = importDocument(registry, document, selected_path);
        result.source_path = selected_path;
        return result;
    } catch (const std::exception& e) {
        addDiagnostic(result, DialogueLoadSeverity::Error, "dialogue_content_invalid_json", selected_path, e.what());
        return result;
    }
}

DialogueProjectLoadResult DialogueProjectLoader::importDocument(DialogueRegistry& registry,
                                                                const nlohmann::json& document,
                                                                const std::filesystem::path& source_path) {
    DialogueProjectLoadResult result;
    result.source_path = source_path;

    if (!document.is_object()) {
        addDiagnostic(result, DialogueLoadSeverity::Error, "dialogue_content_invalid_shape", source_path,
                      "Dialogue project document must be a JSON object.");
        return result;
    }

    if (document.contains("sequences")) {
        const auto& sequences = document["sequences"];
        if (!sequences.is_array()) {
            addDiagnostic(result, DialogueLoadSeverity::Error, "dialogue_sequences_invalid_shape", source_path,
                          "Dialogue sequences field must be an array.");
            return result;
        }

        for (const auto& sequence : sequences) {
            if (!sequence.is_object()) {
                continue;
            }
            const std::string sequence_id = sequence.value("id", "");
            if (sequence_id.empty()) {
                addDiagnostic(result, DialogueLoadSeverity::Warning, "dialogue_sequence_missing_id", source_path,
                              "A dialogue sequence was skipped because it has no id.");
                continue;
            }

            auto nodes = nodesFromSequencePages(sequence.value("pages", nlohmann::json::array()));
            if (nodes.empty()) {
                addDiagnostic(result, DialogueLoadSeverity::Warning, "dialogue_sequence_empty", source_path,
                              "Dialogue sequence '" + sequence_id + "' has no valid pages.");
                continue;
            }

            result.node_count += nodes.size();
            registry.registerConversation(sequence_id, nodes);
            ++result.conversation_count;
        }
    } else {
        const auto before = registry.getConversations().size();
        DialogueDatabase::importProjectDatabase(registry, document);
        result.conversation_count = registry.getConversations().size() - before;
        for (const auto& [id, nodes] : registry.getConversations()) {
            (void)id;
            result.node_count += nodes.size();
        }
    }

    if (result.conversation_count == 0) {
        addDiagnostic(result, DialogueLoadSeverity::Warning, "dialogue_content_empty", source_path,
                      "No valid dialogue conversations were loaded.");
        return result;
    }

    result.loaded = true;
    addDiagnostic(result, DialogueLoadSeverity::Info, "dialogue_content_loaded", source_path,
                  "Loaded " + std::to_string(result.conversation_count) + " dialogue conversation(s).");
    return result;
}

} // namespace urpg::message
