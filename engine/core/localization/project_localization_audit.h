#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::localization {

struct ProjectLocalizationReference {
    std::string key;
    std::filesystem::path document_path;
    std::string owner_kind;
    std::string local_id;
};

struct ProjectLocalizationLocaleKeyGap {
    std::string locale;
    std::string key;
};

// Read-only project localization evidence. Unused keys remain candidates only:
// unindexed owners can still reference a key.
struct ProjectLocalizationAudit {
    std::vector<ProjectLocalizationReference> references;
    std::vector<std::string> available_keys;
    std::vector<std::string> missing_referenced_keys;
    std::vector<std::string> missing_font_profile_locales;
    std::vector<ProjectLocalizationLocaleKeyGap> missing_referenced_locale_keys;
    std::vector<std::string> unused_key_candidates;
    std::vector<std::string> diagnostics;
};

ProjectLocalizationAudit buildProjectLocalizationAudit(const std::filesystem::path& project_root);

} // namespace urpg::localization
