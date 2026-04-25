#include "editor/localization/localization_workspace_model.h"

#include <algorithm>

namespace urpg::editor::localization {

namespace {

std::map<std::string, std::string> readKeys(const nlohmann::json& bundle) {
    std::map<std::string, std::string> keys;
    const auto key_json = bundle.value("keys", nlohmann::json::object());
    for (const auto& [key, value] : key_json.items()) {
        if (value.is_string()) {
            keys[key] = value.get<std::string>();
        }
    }
    return keys;
}

bool containsText(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

} // namespace

void LocalizationWorkspaceModel::loadSource(const nlohmann::json& bundle) {
    source_locale_ = bundle.value("locale", "");
    source_keys_ = readKeys(bundle);
}

void LocalizationWorkspaceModel::loadTarget(const nlohmann::json& bundle) {
    target_locale_ = bundle.value("locale", "");
    target_keys_ = readKeys(bundle);
}

void LocalizationWorkspaceModel::addGlossaryTerm(GlossaryTerm term) {
    glossary_.push_back(std::move(term));
}

void LocalizationWorkspaceModel::setLayoutLimit(const std::string& key, std::size_t max_chars) {
    layout_limits_[key] = max_chars;
}

std::string LocalizationWorkspaceModel::resolve(const std::string& key) const {
    if (target_keys_.contains(key)) {
        return target_keys_.at(key);
    }
    return source_keys_.contains(key) ? source_keys_.at(key) : std::string{};
}

LocalizationWorkspaceSnapshot LocalizationWorkspaceModel::snapshot() const {
    LocalizationWorkspaceSnapshot snapshot;
    snapshot.sourceLocale = source_locale_;
    snapshot.targetLocale = target_locale_;
    for (const auto& [key, source_value] : source_keys_) {
        const auto target = target_keys_.find(key);
        if (target == target_keys_.end()) {
            snapshot.missingKeys.push_back(key);
            continue;
        }
        for (const auto& term : glossary_) {
            if (containsText(source_value, term.source) && !containsText(target->second, term.target)) {
                snapshot.glossaryIssues.push_back(key);
            }
        }
    }
    for (const auto& [key, limit] : layout_limits_) {
        const auto value = resolve(key);
        if (value.size() > limit) {
            snapshot.layoutIssues.push_back(key);
        }
    }
    std::ranges::sort(snapshot.missingKeys);
    std::ranges::sort(snapshot.glossaryIssues);
    std::ranges::sort(snapshot.layoutIssues);
    return snapshot;
}

} // namespace urpg::editor::localization
