#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::community {

struct CommunityFeatureDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct CommunityFeatureAction {
    std::string id;
    std::string label;
    std::string trigger;
    std::string command;
    std::string target;
    std::vector<std::string> required_flags;
    std::map<std::string, std::string> parameters;

    bool operator==(const CommunityFeatureAction& other) const = default;
};

struct CommunityFeatureRuntimeState {
    std::set<std::string> flags;
    std::map<std::string, std::string> variables;
    std::vector<std::string> emitted_commands;
};

struct CommunityFeaturePreview {
    std::string feature_type;
    std::string trigger;
    std::vector<CommunityFeatureAction> active_actions;
    CommunityFeatureRuntimeState resulting_state;
    std::vector<CommunityFeatureDiagnostic> diagnostics;
};

class CommunityWysiwygFeatureDocument {
public:
    std::string schema_version = "urpg.community_wysiwyg.v1";
    std::string id;
    std::string feature_type;
    std::string display_name;
    std::vector<std::string> visual_layers;
    std::vector<CommunityFeatureAction> actions;

    [[nodiscard]] std::vector<CommunityFeatureDiagnostic> validate() const;
    [[nodiscard]] CommunityFeaturePreview preview(const CommunityFeatureRuntimeState& state, const std::string& trigger) const;
    [[nodiscard]] CommunityFeaturePreview execute(CommunityFeatureRuntimeState& state, const std::string& trigger) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static CommunityWysiwygFeatureDocument fromJson(const nlohmann::json& json);
};

std::vector<std::string> communityWysiwygFeatureTypes();
nlohmann::json communityFeaturePreviewToJson(const CommunityFeaturePreview& preview);

} // namespace urpg::community
