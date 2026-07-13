#include "engine/core/project/contextual_creator_project.h"

#include "engine/core/save/save_journal.h"

#include <fstream>

namespace urpg::project {
namespace {

ContextualCreatorProjectResult failure(std::string code, std::string message) {
    return {false, std::move(code), std::move(message), {}};
}

} // namespace

ContextualCreatorProjectResult ContextualCreatorProject::open(const std::filesystem::path& projectRoot) {
    if (projectRoot.empty() || !std::filesystem::is_regular_file(projectRoot / "project.json")) {
        return failure("contextual_project_root_invalid", "Open a valid URPG project before authoring contextual data.");
    }
    project_root_ = projectRoot;
    event_document_ = {};
    dialogues_.clear();
    characters_.clear();
    database_ = {};

    const auto path = project_root_ / kRelativePath;
    if (!std::filesystem::exists(path)) {
        return {true, "contextual_project_opened_empty", "No contextual data exists yet; author a new contextual workflow.", {}};
    }
    try {
        std::ifstream input(path, std::ios::binary);
        const auto json = nlohmann::json::parse(input);
        if (json.value("schema", "") != "urpg.contextual_creator_project.v1") {
            return failure("contextual_project_schema_unsupported", "Contextual project data has an unsupported schema.");
        }
        event_document_ = events::EventDocument::fromJson(json.value("event_document", nlohmann::json::object()));
        const auto serialized_dialogues = json.value("dialogues", nlohmann::json::object());
        for (const auto& [id, dialogue] : serialized_dialogues.items()) {
            dialogues_[id] = dialogue::DialogueGraph::fromJson(dialogue);
        }
        const auto serialized_characters = json.value("characters", nlohmann::json::object());
        for (const auto& [id, character] : serialized_characters.items()) {
            characters_[id] = character::CharacterIdentity::fromJson(character);
        }
        database_ = database::RpgDatabase::fromJson(json.value("database", nlohmann::json::object()));
    } catch (const std::exception& error) {
        return failure("contextual_project_load_failed", error.what());
    }
    const auto validation = validate();
    return {validation.success, validation.success ? "contextual_project_opened" : "contextual_project_validation_failed",
            validation.success ? "Contextual project data was loaded." : validation.message, validation.diagnostics};
}

ContextualCreatorProjectResult ContextualCreatorProject::save() const {
    if (project_root_.empty()) {
        return failure("contextual_project_not_open", "Open a URPG project before saving contextual data.");
    }
    const auto validation = validate();
    if (!validation.success) {
        return validation;
    }
    nlohmann::json dialogues = nlohmann::json::object();
    for (const auto& [id, graph] : dialogues_) {
        dialogues[id] = graph.serialize();
    }
    nlohmann::json characters = nlohmann::json::object();
    for (const auto& [id, identity] : characters_) {
        characters[id] = identity.toJson();
    }
    const nlohmann::json json = {{"schema", "urpg.contextual_creator_project.v1"},
                                 {"event_document", event_document_.toJson()},
                                 {"dialogues", std::move(dialogues)},
                                 {"characters", std::move(characters)},
                                 {"database", database_.toJson()}};
    std::string error;
    if (!SaveJournal::WriteAtomically(project_root_ / kRelativePath, json.dump(2) + "\n", &error)) {
        return failure("contextual_project_save_failed", error);
    }
    return {true, "contextual_project_saved", "Contextual project data was saved atomically.", {}};
}

nlohmann::json ContextualCreatorProject::snapshot() const {
    const auto validation = validate();
    return {{"schema", "urpg.contextual_creator_project_snapshot.v1"},
            {"project_root", project_root_.generic_string()},
            {"event_count", event_document_.events().size()},
            {"dialogue_count", dialogues_.size()},
            {"character_count", characters_.size()},
            {"actor_count", database_.actors().size()},
            {"item_count", database_.items().size()},
            {"is_valid", validation.success},
            {"diagnostics", validation.diagnostics}};
}

void ContextualCreatorProject::setEventDocument(events::EventDocument document) { event_document_ = std::move(document); }
void ContextualCreatorProject::setDialogue(std::string id, dialogue::DialogueGraph graph) { dialogues_[std::move(id)] = std::move(graph); }
void ContextualCreatorProject::setCharacter(std::string id, character::CharacterIdentity identity) { characters_[std::move(id)] = std::move(identity); }
void ContextualCreatorProject::setDatabase(database::RpgDatabase database) { database_ = std::move(database); }

ContextualCreatorProjectResult ContextualCreatorProject::validate() const {
    std::vector<std::string> diagnostics;
    if (project_root_.empty()) diagnostics.push_back("contextual_project_not_open");
    for (const auto& issue : event_document_.validate()) {
        diagnostics.push_back("event:" + issue.code);
    }
    for (const auto& [id, graph] : dialogues_) {
        if (id.empty() || graph.startNode().empty() || graph.findNode(graph.startNode()) == nullptr) {
            diagnostics.push_back("dialogue_invalid:" + id);
        }
    }
    for (const auto& [id, identity] : characters_) {
        if (id.empty() || identity.getName().empty()) diagnostics.push_back("character_invalid:" + id);
    }
    for (const auto& issue : database_.validate()) {
        diagnostics.push_back("database:" + issue.code + ":" + issue.id);
    }
    if (!diagnostics.empty()) {
        return {false, "contextual_project_validation_failed", "Resolve contextual project diagnostics before saving.", std::move(diagnostics)};
    }
    return {true, "contextual_project_valid", "Contextual project data is valid.", {}};
}

} // namespace urpg::project
