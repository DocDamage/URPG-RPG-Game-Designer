#pragma once

#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/quest/quest_objective_graph.h"

#include <nlohmann/json.hpp>

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace urpg::narrative {

struct BranchingQuestDiagnostic {
    std::string code;
    std::string message;
    std::string source_id;
};

struct BranchingQuestPackageClosure {
    std::set<std::string> localization_keys;
    std::set<std::string> voice_asset_ids;
    std::set<std::string> quest_ids;
};

struct BranchingQuestDocument {
    std::string id;
    dialogue::DialogueGraph dialogue;
    quest::QuestObjectiveGraphDocument quest;

    std::vector<BranchingQuestDiagnostic> validate() const;
    BranchingQuestPackageClosure packageClosure() const;
    nlohmann::json toJson() const;
    static std::optional<BranchingQuestDocument> fromJson(const nlohmann::json& json, bool* migrated = nullptr);
};

struct BranchingQuestRuntime {
    std::string document_id;
    std::string dialogue_node_id;
    std::map<std::string, int> dialogue_values;
    std::vector<std::string> dialogue_choice_ids;
    quest::QuestRegistry quest_registry;

    bool start(const BranchingQuestDocument& document);
    bool choose(const BranchingQuestDocument& document, const std::string& choice_id);
    quest::QuestGraphPreview advanceQuest(const BranchingQuestDocument& document, quest::QuestWorldState world,
                                          const std::string& timestamp);
    nlohmann::json save() const;
    static std::optional<BranchingQuestRuntime> load(const nlohmann::json& json);
};

nlohmann::json packageClosureToJson(const BranchingQuestPackageClosure& closure);

} // namespace urpg::narrative
