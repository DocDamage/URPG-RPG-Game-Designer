#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor::localization {

struct GlossaryTerm {
    std::string source;
    std::string target;
};

struct LocalizationWorkspaceSnapshot {
    std::string sourceLocale;
    std::string targetLocale;
    std::vector<std::string> missingKeys;
    std::vector<std::string> glossaryIssues;
    std::vector<std::string> layoutIssues;
};

class LocalizationWorkspaceModel {
public:
    void loadSource(const nlohmann::json& bundle);
    void loadTarget(const nlohmann::json& bundle);
    void addGlossaryTerm(GlossaryTerm term);
    void setLayoutLimit(const std::string& key, std::size_t max_chars);

    [[nodiscard]] std::string resolve(const std::string& key) const;
    [[nodiscard]] LocalizationWorkspaceSnapshot snapshot() const;

private:
    std::string source_locale_;
    std::string target_locale_;
    std::map<std::string, std::string> source_keys_;
    std::map<std::string, std::string> target_keys_;
    std::vector<GlossaryTerm> glossary_;
    std::map<std::string, std::size_t> layout_limits_;
};

} // namespace urpg::editor::localization
