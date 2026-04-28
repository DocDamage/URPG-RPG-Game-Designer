#pragma once

#include "engine/core/localization/locale_catalog.h"
#include "engine/core/message/message_core.h"

#include <nlohmann/json.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::message {

struct DialoguePreviewChoice {
    std::string id;
    std::string label;
    std::string localization_key;
    bool enabled = true;
    std::string disabled_reason;
};

struct DialoguePreviewPage {
    std::string id;
    std::string body;
    std::string localization_key;
    MessagePresentationVariant variant;
    std::vector<DialoguePreviewChoice> choices;
    int32_t default_choice_index = 0;
    bool wait_for_advance = true;
};

struct DialoguePreviewDiagnostic {
    std::string code;
    std::string message;
    std::string page_id;
    std::string target;
};

struct DialoguePreviewDocument {
    std::string id;
    std::string locale = "en-US";
    std::vector<DialoguePreviewPage> pages;
    std::map<int32_t, int32_t> variables;
    std::map<int32_t, std::string> actor_names;
    std::vector<int32_t> party_actor_ids;
    PortraitBindingRegistry portraits;

    nlohmann::json toJson() const;
    std::vector<DialoguePreviewDiagnostic> validate(const localization::LocaleCatalog& locale_catalog) const;

    static DialoguePreviewDocument fromJson(const nlohmann::json& json);
};

struct DialoguePreviewResolvedChoice {
    std::string id;
    std::string label;
    bool enabled = true;
    std::string disabled_reason;
};

struct DialoguePreviewResult {
    std::string page_id;
    std::string locale;
    std::string speaker;
    std::string body;
    std::optional<PortraitBinding> portrait;
    std::vector<DialoguePreviewResolvedChoice> choices;
    RichTextLayoutMetrics layout_metrics;
    MessageFlowSnapshot flow_snapshot;
    std::vector<DialoguePreviewDiagnostic> diagnostics;
};

DialoguePreviewResult PreviewDialoguePage(const DialoguePreviewDocument& document,
                                          const localization::LocaleCatalog& locale_catalog,
                                          const std::string& page_id);

} // namespace urpg::message
