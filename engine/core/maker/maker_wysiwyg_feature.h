#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::maker {

struct MakerFeatureDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct MakerFeatureAction {
    std::string id;
    std::string label;
    std::string trigger;
    std::string operation;
    std::string target;
    std::vector<std::string> required_flags;
    std::map<std::string, std::string> parameters;

    bool operator==(const MakerFeatureAction& other) const = default;
};

struct MakerFeatureRuntimeState {
    std::set<std::string> flags;
    std::map<std::string, std::string> variables;
    std::vector<std::string> emitted_operations;
};

struct MakerFeaturePreview {
    std::string feature_type;
    std::string trigger;
    std::vector<MakerFeatureAction> active_actions;
    MakerFeatureRuntimeState resulting_state;
    std::vector<MakerFeatureDiagnostic> diagnostics;
};

class MakerWysiwygFeatureDocument {
public:
    std::string schema_version = "urpg.maker_wysiwyg.v1";
    std::string id;
    std::string feature_type;
    std::string display_name;
    std::vector<std::string> visual_layers;
    std::vector<MakerFeatureAction> actions;

    [[nodiscard]] std::vector<MakerFeatureDiagnostic> validate() const;
    [[nodiscard]] MakerFeaturePreview preview(const MakerFeatureRuntimeState& state, const std::string& trigger) const;
    [[nodiscard]] MakerFeaturePreview execute(MakerFeatureRuntimeState& state, const std::string& trigger) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static MakerWysiwygFeatureDocument fromJson(const nlohmann::json& json);
};

std::vector<std::string> makerWysiwygFeatureTypes();
nlohmann::json makerFeaturePreviewToJson(const MakerFeaturePreview& preview);

} // namespace urpg::maker
