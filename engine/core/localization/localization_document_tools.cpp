#include "engine/core/localization/localization_document_tools.h"

#include <stdexcept>
#include <string>

namespace urpg::localization {

namespace {

void addIfLocalized(nlohmann::json& keys, const nlohmann::json& object, const char* textField) {
    if (!object.is_object()) {
        return;
    }
    const std::string key = object.value("localization_key", "");
    if (key.empty()) {
        return;
    }
    keys[key] = object.value(textField, "");
}

} // namespace

nlohmann::json extractDialoguePreviewLocalizationBundle(const nlohmann::json& document) {
    if (!document.is_object()) {
        throw std::invalid_argument("Dialogue preview document must be a JSON object.");
    }

    nlohmann::json bundle;
    bundle["locale"] = document.value("locale", "en-US");
    bundle["keys"] = nlohmann::json::object();

    if (!document.contains("pages") || !document["pages"].is_array()) {
        return bundle;
    }

    for (const auto& page : document["pages"]) {
        addIfLocalized(bundle["keys"], page, "body");
        if (!page.contains("choices") || !page["choices"].is_array()) {
            continue;
        }
        for (const auto& choice : page["choices"]) {
            addIfLocalized(bundle["keys"], choice, "label");
        }
    }

    return bundle;
}

nlohmann::json writebackDialoguePreviewLocalizationBundle(const nlohmann::json& document,
                                                          const nlohmann::json& bundle) {
    if (!document.is_object()) {
        throw std::invalid_argument("Dialogue preview document must be a JSON object.");
    }
    if (!bundle.is_object() || !bundle.contains("keys") || !bundle["keys"].is_object()) {
        throw std::invalid_argument("Localization bundle must contain a keys object.");
    }

    nlohmann::json updated = document;
    if (!updated.contains("pages") || !updated["pages"].is_array()) {
        return updated;
    }

    const auto& keys = bundle["keys"];
    for (auto& page : updated["pages"]) {
        const std::string pageKey = page.value("localization_key", "");
        if (!pageKey.empty() && keys.contains(pageKey) && keys[pageKey].is_string()) {
            page["body"] = keys[pageKey];
        }

        if (!page.contains("choices") || !page["choices"].is_array()) {
            continue;
        }
        for (auto& choice : page["choices"]) {
            const std::string choiceKey = choice.value("localization_key", "");
            if (!choiceKey.empty() && keys.contains(choiceKey) && keys[choiceKey].is_string()) {
                choice["label"] = keys[choiceKey];
            }
        }
    }

    return updated;
}

} // namespace urpg::localization
