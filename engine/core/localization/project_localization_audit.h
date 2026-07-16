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

struct ProjectDialogueMediaReference {
    std::filesystem::path document_path;
    std::string node_id;
    std::string voice_asset_id;
    std::string caption_key;
    std::string voice_take_locale;
    std::string voice_take_id;
    std::string muted_alternative_asset_id;
    bool voice_asset_attached = false;
    bool caption_key_available = false;
    bool voice_take_metadata_present = false;
    bool voice_take_metadata_valid = false;
    bool muted_alternative_asset_attached = false;
};

struct ProjectDialogueMediaIssue {
    std::string code;
    std::filesystem::path document_path;
    std::string node_id;
    std::string voice_asset_id;
    std::string caption_key;
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
    std::vector<ProjectDialogueMediaReference> dialogue_media_references;
    std::vector<ProjectDialogueMediaIssue> dialogue_media_issues;
    std::vector<std::string> diagnostics;
};

ProjectLocalizationAudit buildProjectLocalizationAudit(const std::filesystem::path& project_root);

} // namespace urpg::localization
