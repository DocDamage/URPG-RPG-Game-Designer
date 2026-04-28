#include "engine/core/message/dialogue_preview.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <utility>

namespace urpg::message {
namespace {

std::string modeToString(MessagePresentationMode mode) {
    switch (mode) {
    case MessagePresentationMode::Speaker:
        return "speaker";
    case MessagePresentationMode::Narration:
        return "narration";
    case MessagePresentationMode::System:
        return "system";
    }
    return "speaker";
}

std::string toneToString(MessageTone tone) {
    switch (tone) {
    case MessageTone::Portrait:
        return "portrait";
    case MessageTone::Neutral:
        return "neutral";
    case MessageTone::System:
        return "system";
    }
    return "portrait";
}

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

nlohmann::json variantToJson(const MessagePresentationVariant& variant) {
    return {
        {"mode", modeToString(variant.mode)},
        {"tone", toneToString(variant.tone)},
        {"speaker", variant.speaker},
        {"face_actor_id", variant.face_actor_id},
        {"route_token", variant.route_token},
    };
}

MessagePresentationVariant variantFromJson(const nlohmann::json& json) {
    MessagePresentationVariant variant;
    if (!json.is_object()) {
        return variant;
    }
    variant.mode = modeFromString(json.value("mode", "speaker"));
    variant.tone = toneFromString(json.value("tone", "portrait"));
    variant.speaker = json.value("speaker", "");
    variant.face_actor_id = json.value("face_actor_id", 0);
    variant.route_token = json.value("route_token", "");
    return variant;
}

std::string resolveLocalizedText(const localization::LocaleCatalog& catalog, const std::string& key,
                                 const std::string& fallback) {
    if (!key.empty()) {
        if (auto localized = catalog.getKey(key); localized.has_value()) {
            return *localized;
        }
    }
    return fallback;
}

std::set<int32_t> referencedVariables(const std::string& text) {
    std::set<int32_t> ids;
    for (size_t i = 0; i + 3 < text.size(); ++i) {
        if (text[i] != '\\' || static_cast<char>(std::toupper(static_cast<unsigned char>(text[i + 1]))) != 'V' ||
            text[i + 2] != '[') {
            continue;
        }
        size_t cursor = i + 3;
        int32_t value = 0;
        bool found_digit = false;
        while (cursor < text.size() && std::isdigit(static_cast<unsigned char>(text[cursor]))) {
            value = value * 10 + static_cast<int32_t>(text[cursor] - '0');
            found_digit = true;
            ++cursor;
        }
        if (found_digit && cursor < text.size() && text[cursor] == ']') {
            ids.insert(value);
            i = cursor;
        }
    }
    return ids;
}

const DialoguePreviewPage* findPage(const DialoguePreviewDocument& document, const std::string& page_id) {
    if (!page_id.empty()) {
        const auto it = std::find_if(document.pages.begin(), document.pages.end(), [&page_id](const auto& page) {
            return page.id == page_id;
        });
        if (it != document.pages.end()) {
            return &(*it);
        }
    }
    return document.pages.empty() ? nullptr : &document.pages.front();
}

DialoguePage toRuntimePage(const DialoguePreviewPage& page, const localization::LocaleCatalog& catalog,
                           const RichTextLayoutEngine& layout_engine) {
    DialoguePage runtime;
    runtime.id = page.id;
    runtime.body = layout_engine.resolveEscapes(resolveLocalizedText(catalog, page.localization_key, page.body));
    runtime.variant = page.variant;
    runtime.wait_for_advance = page.wait_for_advance;
    runtime.default_choice_index = page.default_choice_index;
    for (const auto& choice : page.choices) {
        runtime.choices.push_back({
            choice.id,
            layout_engine.resolveEscapes(resolveLocalizedText(catalog, choice.localization_key, choice.label)),
            choice.enabled,
            choice.disabled_reason,
        });
    }
    return runtime;
}

} // namespace

nlohmann::json DialoguePreviewDocument::toJson() const {
    nlohmann::json json;
    json["schema"] = "urpg.dialogue_preview.v1";
    json["id"] = id;
    json["locale"] = locale;
    json["variables"] = nlohmann::json::object();
    for (const auto& [key, value] : variables) {
        json["variables"][std::to_string(key)] = value;
    }
    json["actor_names"] = nlohmann::json::object();
    for (const auto& [key, value] : actor_names) {
        json["actor_names"][std::to_string(key)] = value;
    }
    json["party_actor_ids"] = party_actor_ids;
    json["pages"] = nlohmann::json::array();
    for (const auto& page : pages) {
        nlohmann::json page_json;
        page_json["id"] = page.id;
        page_json["body"] = page.body;
        page_json["localization_key"] = page.localization_key;
        page_json["variant"] = variantToJson(page.variant);
        page_json["default_choice_index"] = page.default_choice_index;
        page_json["wait_for_advance"] = page.wait_for_advance;
        page_json["choices"] = nlohmann::json::array();
        for (const auto& choice : page.choices) {
            page_json["choices"].push_back({
                {"id", choice.id},
                {"label", choice.label},
                {"localization_key", choice.localization_key},
                {"enabled", choice.enabled},
                {"disabled_reason", choice.disabled_reason},
            });
        }
        if (const auto* binding = portraits.resolveBinding(page.variant.face_actor_id)) {
            page_json["portrait"] = {
                {"face_name", binding->face_name},
                {"face_index", binding->face_index},
                {"mirror", binding->mirror},
            };
        }
        json["pages"].push_back(std::move(page_json));
    }
    return json;
}

std::vector<DialoguePreviewDiagnostic> DialoguePreviewDocument::validate(
    const localization::LocaleCatalog& locale_catalog) const {
    std::vector<DialoguePreviewDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_preview_id", "Dialogue preview requires an id.", "", ""});
    }
    if (pages.empty()) {
        diagnostics.push_back({"missing_pages", "Dialogue preview requires at least one page.", "", ""});
    }

    std::set<std::string> page_ids;
    for (const auto& page : pages) {
        if (page.id.empty()) {
            diagnostics.push_back({"missing_page_id", "Dialogue preview page requires an id.", "", ""});
        } else if (!page_ids.insert(page.id).second) {
            diagnostics.push_back({"duplicate_page_id", "Dialogue preview page id must be unique.", page.id, page.id});
        }
        if (page.localization_key.empty() && page.body.empty()) {
            diagnostics.push_back({"missing_body", "Dialogue preview page requires body text or localization.", page.id,
                                   ""});
        }
        if (!page.localization_key.empty() && !locale_catalog.hasKey(page.localization_key)) {
            diagnostics.push_back({"missing_localization_key", "Dialogue preview page localization key is missing.",
                                   page.id, page.localization_key});
        }
        if (page.variant.mode == MessagePresentationMode::Speaker && page.variant.speaker.empty()) {
            diagnostics.push_back({"missing_speaker", "Speaker dialogue page requires a speaker.", page.id, ""});
        }
        if (page.variant.tone == MessageTone::Portrait && page.variant.face_actor_id > 0 &&
            portraits.resolveBinding(page.variant.face_actor_id) == nullptr) {
            diagnostics.push_back({"missing_portrait_binding", "Portrait dialogue page requires a portrait binding.",
                                   page.id, std::to_string(page.variant.face_actor_id)});
        }
        const auto text = resolveLocalizedText(locale_catalog, page.localization_key, page.body);
        for (const auto variable_id : referencedVariables(text)) {
            if (!variables.contains(variable_id)) {
                diagnostics.push_back({"missing_variable", "Dialogue preview references an unresolved variable.",
                                       page.id, std::to_string(variable_id)});
            }
        }
        if (!page.choices.empty() &&
            std::none_of(page.choices.begin(), page.choices.end(), [](const auto& choice) { return choice.enabled; })) {
            diagnostics.push_back({"choice_unreachable", "Dialogue preview page has no enabled choices.", page.id, ""});
        }
        for (const auto& choice : page.choices) {
            if (choice.id.empty()) {
                diagnostics.push_back({"missing_choice_id", "Dialogue preview choice requires an id.", page.id, ""});
            }
            if (choice.localization_key.empty() && choice.label.empty()) {
                diagnostics.push_back({"missing_choice_label", "Dialogue preview choice requires a label.", page.id,
                                       choice.id});
            }
            if (!choice.localization_key.empty() && !locale_catalog.hasKey(choice.localization_key)) {
                diagnostics.push_back({"missing_localization_key", "Dialogue preview choice localization key is missing.",
                                       page.id, choice.localization_key});
                if (choice.label.empty()) {
                    diagnostics.push_back({"missing_choice_label", "Dialogue preview choice has no resolved label.",
                                           page.id, choice.id});
                }
            }
        }
    }
    return diagnostics;
}

DialoguePreviewDocument DialoguePreviewDocument::fromJson(const nlohmann::json& json) {
    DialoguePreviewDocument document;
    if (!json.is_object()) {
        return document;
    }
    document.id = json.value("id", "");
    document.locale = json.value("locale", "en-US");
    if (const auto vars = json.find("variables"); vars != json.end() && vars->is_object()) {
        for (const auto& [key, value] : vars->items()) {
            document.variables[std::stoi(key)] = value.get<int32_t>();
        }
    }
    if (const auto actors = json.find("actor_names"); actors != json.end() && actors->is_object()) {
        for (const auto& [key, value] : actors->items()) {
            document.actor_names[std::stoi(key)] = value.get<std::string>();
        }
    }
    for (const auto& actor_id : json.value("party_actor_ids", nlohmann::json::array())) {
        document.party_actor_ids.push_back(actor_id.get<int32_t>());
    }
    for (const auto& page_json : json.value("pages", nlohmann::json::array())) {
        if (!page_json.is_object()) {
            continue;
        }
        DialoguePreviewPage page;
        page.id = page_json.value("id", "");
        page.body = page_json.value("body", "");
        page.localization_key = page_json.value("localization_key", "");
        page.variant = variantFromJson(page_json.value("variant", nlohmann::json::object()));
        page.default_choice_index = page_json.value("default_choice_index", 0);
        page.wait_for_advance = page_json.value("wait_for_advance", true);
        for (const auto& choice_json : page_json.value("choices", nlohmann::json::array())) {
            if (!choice_json.is_object()) {
                continue;
            }
            page.choices.push_back({
                choice_json.value("id", ""),
                choice_json.value("label", ""),
                choice_json.value("localization_key", ""),
                choice_json.value("enabled", true),
                choice_json.value("disabled_reason", ""),
            });
        }
        if (const auto portrait = page_json.find("portrait"); portrait != page_json.end() && portrait->is_object()) {
            document.portraits.registerBinding(page.variant.face_actor_id,
                                               {portrait->value("face_name", ""),
                                                portrait->value("face_index", 0),
                                                portrait->value("mirror", false)});
        }
        document.pages.push_back(std::move(page));
    }
    return document;
}

DialoguePreviewResult PreviewDialoguePage(const DialoguePreviewDocument& document,
                                          const localization::LocaleCatalog& locale_catalog,
                                          const std::string& page_id) {
    DialoguePreviewResult result;
    result.locale = locale_catalog.getLocaleCode().empty() ? document.locale : locale_catalog.getLocaleCode();
    result.diagnostics = document.validate(locale_catalog);

    const auto* page = findPage(document, page_id);
    if (page == nullptr) {
        result.diagnostics.push_back({"missing_preview_page", "Requested dialogue preview page does not exist.",
                                      page_id, page_id});
        return result;
    }

    RichTextLayoutEngine layout_engine;
    layout_engine.setMaxWidth(640);
    layout_engine.setVariableResolver([&document](int32_t id) {
        const auto it = document.variables.find(id);
        return it == document.variables.end() ? 0 : it->second;
    });
    layout_engine.setActorNameResolver([&document](int32_t id) {
        const auto it = document.actor_names.find(id);
        return it == document.actor_names.end() ? std::string("Actor ") + std::to_string(id) : it->second;
    });
    layout_engine.setPartyMemberResolver([&document](int32_t index) {
        if (index < 0 || static_cast<size_t>(index) >= document.party_actor_ids.size()) {
            return 0;
        }
        return document.party_actor_ids[static_cast<size_t>(index)];
    });

    std::vector<DialoguePage> runtime_pages;
    runtime_pages.reserve(document.pages.size());
    for (const auto& source_page : document.pages) {
        runtime_pages.push_back(toRuntimePage(source_page, locale_catalog, layout_engine));
    }

    MessageFlowRunner runner;
    runner.begin(runtime_pages);
    const auto selected_index = static_cast<size_t>(std::distance(
        document.pages.data(), page));
    if (selected_index < runtime_pages.size()) {
        runner.restore({selected_index, MessageFlowState::Presenting, static_cast<size_t>(page->default_choice_index)});
    }
    runner.markPagePresented();

    result.page_id = page->id;
    result.speaker = page->variant.speaker;
    result.body = layout_engine.resolveEscapes(resolveLocalizedText(locale_catalog, page->localization_key, page->body));
    if (const auto* binding = document.portraits.resolveBinding(page->variant.face_actor_id)) {
        result.portrait = *binding;
    }
    const auto layout = layout_engine.layout(result.body);
    result.layout_metrics = layout.metrics;
    result.flow_snapshot = runner.snapshot();

    const auto* runtime_page = runner.currentPage();
    if (runtime_page != nullptr) {
        for (const auto& choice : runtime_page->choices) {
            result.choices.push_back({choice.id, choice.label, choice.enabled, choice.disabled_reason});
        }
    }
    return result;
}

} // namespace urpg::message
