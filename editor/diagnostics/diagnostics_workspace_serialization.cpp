#include "editor/diagnostics/diagnostics_workspace.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/ui/menu_serializer.h"

#include <nlohmann/json.hpp>

#include <fstream>

namespace urpg::editor {

namespace {

const char* MessagePresentationModeName(urpg::message::MessagePresentationMode mode) {
    switch (mode) {
    case urpg::message::MessagePresentationMode::Speaker:
        return "speaker";
    case urpg::message::MessagePresentationMode::Narration:
        return "narration";
    case urpg::message::MessagePresentationMode::System:
        return "system";
    }
    return "speaker";
}

std::optional<urpg::message::MessagePresentationMode> MessagePresentationModeFromString(const std::string& s) {
    if (s == "speaker") {
        return urpg::message::MessagePresentationMode::Speaker;
    }
    if (s == "narration") {
        return urpg::message::MessagePresentationMode::Narration;
    }
    if (s == "system") {
        return urpg::message::MessagePresentationMode::System;
    }
    return std::nullopt;
}

const char* MessageToneName(urpg::message::MessageTone tone) {
    switch (tone) {
    case urpg::message::MessageTone::Portrait:
        return "portrait";
    case urpg::message::MessageTone::Neutral:
        return "neutral";
    case urpg::message::MessageTone::System:
        return "system";
    }
    return "portrait";
}

std::optional<urpg::message::MessageTone> MessageToneFromString(const std::string& s) {
    if (s == "portrait") {
        return urpg::message::MessageTone::Portrait;
    }
    if (s == "neutral") {
        return urpg::message::MessageTone::Neutral;
    }
    if (s == "system") {
        return urpg::message::MessageTone::System;
    }
    return std::nullopt;
}

nlohmann::json ChoiceOptionJson(const urpg::message::ChoiceOption& option) {
    return {
        {"id", option.id},
        {"label", option.label},
        {"enabled", option.enabled},
        {"disabled_reason", option.disabled_reason},
    };
}

nlohmann::json DialoguePageJson(const urpg::message::DialoguePage& page) {
    nlohmann::json choices = nlohmann::json::array();
    for (const auto& choice : page.choices) {
        choices.push_back(ChoiceOptionJson(choice));
    }
    return {
        {"id", page.id},
        {"body", page.body},
        {"speaker", page.variant.speaker},
        {"mode", MessagePresentationModeName(page.variant.mode)},
        {"tone", MessageToneName(page.variant.tone)},
        {"face_actor_id", page.variant.face_actor_id},
        {"wait_for_advance", page.wait_for_advance},
        {"choices", std::move(choices)},
        {"default_choice_index", page.default_choice_index},
        {"command", page.command},
    };
}

std::optional<urpg::message::DialoguePage> DialoguePageFromJson(const nlohmann::json& j) {
    if (!j.is_object()) {
        return std::nullopt;
    }

    urpg::message::DialoguePage page;
    if (j.contains("id") && j["id"].is_string()) {
        page.id = j["id"].get<std::string>();
    }
    if (j.contains("body") && j["body"].is_string()) {
        page.body = j["body"].get<std::string>();
    }
    if (j.contains("speaker") && j["speaker"].is_string()) {
        page.variant.speaker = j["speaker"].get<std::string>();
    }
    if (j.contains("mode") && j["mode"].is_string()) {
        const auto mode = MessagePresentationModeFromString(j["mode"].get<std::string>());
        if (mode.has_value()) {
            page.variant.mode = *mode;
        }
    }
    if (j.contains("tone") && j["tone"].is_string()) {
        const auto tone = MessageToneFromString(j["tone"].get<std::string>());
        if (tone.has_value()) {
            page.variant.tone = *tone;
        }
    }
    if (j.contains("face_actor_id") && j["face_actor_id"].is_number_integer()) {
        page.variant.face_actor_id = j["face_actor_id"].get<int32_t>();
    }
    if (j.contains("wait_for_advance") && j["wait_for_advance"].is_boolean()) {
        page.wait_for_advance = j["wait_for_advance"].get<bool>();
    }
    if (j.contains("choices") && j["choices"].is_array()) {
        for (const auto& choice_j : j["choices"]) {
            if (!choice_j.is_object()) {
                continue;
            }

            urpg::message::ChoiceOption option;
            if (choice_j.contains("id") && choice_j["id"].is_string()) {
                option.id = choice_j["id"].get<std::string>();
            }
            if (choice_j.contains("label") && choice_j["label"].is_string()) {
                option.label = choice_j["label"].get<std::string>();
            }
            if (choice_j.contains("enabled") && choice_j["enabled"].is_boolean()) {
                option.enabled = choice_j["enabled"].get<bool>();
            }
            if (choice_j.contains("disabled_reason") && choice_j["disabled_reason"].is_string()) {
                option.disabled_reason = choice_j["disabled_reason"].get<std::string>();
            }
            page.choices.push_back(std::move(option));
        }
    }
    if (j.contains("default_choice_index") && j["default_choice_index"].is_number_integer()) {
        page.default_choice_index = j["default_choice_index"].get<int32_t>();
    }
    if (j.contains("command") && j["command"].is_string()) {
        page.command = j["command"].get<std::string>();
    }

    return page;
}

} // namespace

std::string DiagnosticsWorkspace::exportMessageStateJson() const {
    const auto& pages = message_panel_.getModel().pages();
    nlohmann::json result = nlohmann::json::array();
    for (const auto& page : pages) {
        result.push_back(DialoguePageJson(page));
    }
    return result.dump();
}

bool DiagnosticsWorkspace::saveMessageStateToFile(const std::string& path) const {
    std::ofstream ofs(path);
    if (!ofs) {
        return false;
    }

    ofs << exportMessageStateJson();
    return ofs.good();
}

bool DiagnosticsWorkspace::loadMessageStateFromFile(const std::string& path, urpg::message::MessageFlowRunner& runner) {
    std::ifstream ifs(path);
    if (!ifs) {
        return false;
    }

    nlohmann::json j;
    try {
        ifs >> j;
    } catch (...) {
        return false;
    }

    if (!j.is_array()) {
        return false;
    }

    std::vector<urpg::message::DialoguePage> pages;
    for (const auto& page_j : j) {
        const auto page = DialoguePageFromJson(page_j);
        if (page.has_value()) {
            pages.push_back(*page);
        }
    }

    runner.resetWithPages(std::move(pages));
    message_panel_.update();
    refreshMessageInspectorSnapshotIfActive();
    return true;
}

std::string DiagnosticsWorkspace::exportMenuStateJson() const {
    if (!menu_scene_graph_) {
        return "{}";
    }

    return urpg::ui::MenuSceneSerializer::SerializeGraph(*menu_scene_graph_).dump();
}

bool DiagnosticsWorkspace::saveMenuStateToFile(const std::string& path) {
    if (!menu_scene_graph_) {
        return false;
    }

    std::ofstream ofs(path);
    if (!ofs) {
        return false;
    }

    ofs << urpg::ui::MenuSceneSerializer::SerializeGraph(*menu_scene_graph_).dump(2);
    return ofs.good();
}

bool DiagnosticsWorkspace::loadMenuStateFromFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        return false;
    }

    nlohmann::json j;
    try {
        ifs >> j;
    } catch (...) {
        return false;
    }

    if (!menu_scene_graph_) {
        return false;
    }

    if (j.contains("scenes") && j["scenes"].is_array()) {
        return urpg::ui::MenuSceneSerializer::DeserializeGraph(j, *menu_scene_graph_);
    }
    return urpg::ui::MenuSceneSerializer::Deserialize(j, *menu_scene_graph_);
}

std::string DiagnosticsWorkspace::exportAbilityDraftStateJson() const {
    const nlohmann::json result = ability_panel_.getDraftAsset();
    return result.dump(2);
}

bool DiagnosticsWorkspace::saveAbilityDraftStateToFile(const std::string& path) const {
    return urpg::ability::saveAuthoredAbilityAssetToFile(
        ability_panel_.getDraftAsset(),
        std::filesystem::path(path));
}

bool DiagnosticsWorkspace::loadAbilityDraftStateFromFile(const std::string& path) {
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(std::filesystem::path(path));
    if (!asset.has_value()) {
        return false;
    }

    ability_panel_.setDraftFromAsset(*asset);

    rebuildAbilityDraftPreviewRuntime();
    return true;
}

std::string DiagnosticsWorkspace::exportMigrationWizardReportJson() const {
    return migration_wizard_panel_.getModel()->getReportJson();
}

bool DiagnosticsWorkspace::saveMigrationWizardReportToFile(const std::string& path) {
    return migration_wizard_panel_.saveReportToFile(path);
}

bool DiagnosticsWorkspace::loadMigrationWizardReportFromFile(const std::string& path) {
    const bool loaded = migration_wizard_panel_.loadReportFromFile(path);
    refreshMigrationWizardSnapshotIfActive();
    return loaded;
}

} // namespace urpg::editor
