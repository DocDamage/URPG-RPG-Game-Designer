#include "engine/core/narrative/branching_quest_document.h"

#include <algorithm>

namespace urpg::narrative {

std::vector<BranchingQuestDiagnostic> BranchingQuestDocument::validate() const {
    std::vector<BranchingQuestDiagnostic> result;
    if (id.empty()) {
        result.push_back({"missing_document_id", "Branching quest requires a stable document id.", ""});
    }
    for (const auto& diagnostic : dialogue.validate()) {
        result.push_back({diagnostic.code, diagnostic.message, diagnostic.node_id});
    }
    for (const auto& diagnostic : dialogue.analyzeFlow()) {
        result.push_back({diagnostic.code, diagnostic.message, diagnostic.node_id});
    }
    for (const auto& diagnostic : quest.validate()) {
        result.push_back({diagnostic.code, diagnostic.message, diagnostic.node_id});
    }
    for (const auto& diagnostic : quest.analyzeFlow()) {
        result.push_back({diagnostic.code, diagnostic.message, diagnostic.node_id});
    }

    std::set<std::string> choice_ids;
    for (const auto& [_, node] : dialogue.nodes()) {
        for (const auto& choice : node.choices) {
            choice_ids.insert(choice.id);
        }
    }
    for (const auto& node : quest.nodes) {
        for (const auto& condition : node.conditions) {
            if (condition.type == "dialogue_choice" && !choice_ids.contains(condition.id)) {
                result.push_back({"missing_dialogue_choice", "Quest condition references an unknown dialogue choice.",
                                  node.id});
            }
        }
    }
    return result;
}

BranchingQuestPackageClosure BranchingQuestDocument::packageClosure() const {
    BranchingQuestPackageClosure closure;
    if (!quest.quest_id.empty()) {
        closure.quest_ids.insert(quest.quest_id);
    }
    for (const auto& [_, node] : dialogue.nodes()) {
        if (!node.localization_key.empty()) closure.localization_keys.insert(node.localization_key);
        if (!node.caption_localization_key.empty()) closure.localization_keys.insert(node.caption_localization_key);
        if (!node.voice_asset_id.empty()) closure.voice_asset_ids.insert(node.voice_asset_id);
        for (const auto& choice : node.choices) {
            if (!choice.localization_key.empty()) closure.localization_keys.insert(choice.localization_key);
        }
    }
    for (const auto& node : quest.nodes) {
        if (!node.localization_key.empty()) closure.localization_keys.insert(node.localization_key);
    }
    return closure;
}

nlohmann::json BranchingQuestDocument::toJson() const {
    return {{"schema_version", "urpg.branching_quest.v2"},
            {"id", id},
            {"dialogue_graph", dialogue.serialize()},
            {"quest_graph", quest.toJson()},
            {"package_closure", packageClosureToJson(packageClosure())}};
}

std::optional<BranchingQuestDocument> BranchingQuestDocument::fromJson(const nlohmann::json& json, bool* migrated) {
    if (!json.is_object()) return std::nullopt;
    const bool legacy = !json.contains("schema_version") || json.value("schema_version", "") == "urpg.branching_quest.v1";
    const char* dialogue_key = json.contains("dialogue_graph") ? "dialogue_graph" : "dialogue";
    const char* quest_key = json.contains("quest_graph") ? "quest_graph" : "quest";
    if (!json.contains(dialogue_key) || !json.contains(quest_key)) return std::nullopt;
    auto restored_dialogue = dialogue::DialogueGraph::fromJson(json.at(dialogue_key));
    if (!restored_dialogue) return std::nullopt;
    BranchingQuestDocument document;
    document.id = json.value("id", json.value("quest_id", ""));
    document.dialogue = std::move(*restored_dialogue);
    document.quest = quest::QuestObjectiveGraphDocument::fromJson(json.at(quest_key));
    if (migrated != nullptr) *migrated = legacy || dialogue_key == std::string("dialogue");
    return document;
}

bool BranchingQuestRuntime::start(const BranchingQuestDocument& document) {
    if (!document.validate().empty()) return false;
    document_id = document.id;
    dialogue_node_id = document.dialogue.startNode();
    dialogue_values.clear();
    dialogue_choice_ids.clear();
    quest_registry = quest::QuestRegistry{};
    quest_registry.registerQuest(document.quest.toQuestDefinition());
    return true;
}

bool BranchingQuestRuntime::choose(const BranchingQuestDocument& document, const std::string& choice_id) {
    if (document.id != document_id) return false;
    const auto transition = document.dialogue.previewChoice(dialogue_node_id, choice_id, dialogue_values);
    if (!transition.applied) return false;
    dialogue_node_id = transition.next_node_id;
    dialogue_values = transition.values;
    dialogue_choice_ids.push_back(choice_id);
    return true;
}

quest::QuestGraphPreview BranchingQuestRuntime::advanceQuest(const BranchingQuestDocument& document,
                                                              quest::QuestWorldState world,
                                                              const std::string& timestamp) {
    world.dialogue_choices.insert(world.dialogue_choices.end(), dialogue_choice_ids.begin(), dialogue_choice_ids.end());
    return document.quest.applyReadyObjectives(quest_registry, world, timestamp);
}

nlohmann::json BranchingQuestRuntime::save() const {
    return {{"schema_version", "urpg.branching_quest_runtime.v1"},
            {"document_id", document_id},
            {"dialogue_node_id", dialogue_node_id},
            {"dialogue_values", dialogue_values},
            {"dialogue_choice_ids", dialogue_choice_ids},
            {"quest_registry", quest_registry.serialize()}};
}

std::optional<BranchingQuestRuntime> BranchingQuestRuntime::load(const nlohmann::json& json) {
    try {
        if (!json.is_object() || json.value("schema_version", "") != "urpg.branching_quest_runtime.v1") {
            return std::nullopt;
        }
        BranchingQuestRuntime runtime;
        runtime.document_id = json.at("document_id").get<std::string>();
        runtime.dialogue_node_id = json.at("dialogue_node_id").get<std::string>();
        runtime.dialogue_values = json.at("dialogue_values").get<std::map<std::string, int>>();
        runtime.dialogue_choice_ids = json.at("dialogue_choice_ids").get<std::vector<std::string>>();
        runtime.quest_registry = quest::QuestRegistry::deserialize(json.at("quest_registry"));
        return runtime;
    } catch (const nlohmann::json::exception&) {
        return std::nullopt;
    }
}

nlohmann::json packageClosureToJson(const BranchingQuestPackageClosure& closure) {
    return {{"localization_keys", closure.localization_keys},
            {"voice_asset_ids", closure.voice_asset_ids},
            {"quest_ids", closure.quest_ids}};
}

} // namespace urpg::narrative
