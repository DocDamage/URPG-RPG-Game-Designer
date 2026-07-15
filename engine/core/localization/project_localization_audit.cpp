#include "engine/core/localization/project_localization_audit.h"

#include "engine/core/localization/locale_catalog.h"

#include <algorithm>
#include <functional>
#include <fstream>
#include <iterator>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <tuple>

namespace urpg::localization {
namespace {

void sortUnique(std::vector<std::string>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

void scanDirectory(const std::filesystem::path& directory, const char* unreadableDiagnostic,
                   const std::function<void(const std::filesystem::directory_entry&)>& visitor,
                   std::vector<std::string>& diagnostics) {
    std::error_code error;
    std::filesystem::directory_iterator iterator(directory, error);
    if (error) {
        diagnostics.push_back(unreadableDiagnostic);
        return;
    }
    for (const auto& entry : iterator) {
        if (error) {
            diagnostics.push_back(unreadableDiagnostic);
            return;
        }
        if (entry.is_regular_file() && entry.path().extension() == ".json") visitor(entry);
    }
}

} // namespace

ProjectLocalizationAudit buildProjectLocalizationAudit(const std::filesystem::path& project_root) {
    ProjectLocalizationAudit audit;
    if (project_root.empty()) {
        audit.diagnostics.push_back("project_localization_audit_project_root_missing");
        return audit;
    }

    std::set<std::string> available;
    std::map<std::string, std::set<std::string>> keys_by_locale;
    scanDirectory(project_root / "content" / "localization", "project_localization_audit_bundles_unreadable",
                  [&](const auto& entry) {
                      std::ifstream input(entry.path(), std::ios::binary);
                      const auto bundle = nlohmann::json::parse(input, nullptr, false);
                      if (!LocaleCatalog::validateBundleJson(bundle)) {
                          audit.diagnostics.push_back("project_localization_audit_bundle_invalid:" +
                                                      entry.path().filename().string());
                          return;
                      }
                      LocaleCatalog catalog;
                      catalog.loadFromJson(bundle);
                      const auto locale = catalog.getLocaleCode().empty() ? std::string{"<missing locale>"}
                                                                         : catalog.getLocaleCode();
                      if (catalog.getLocaleCode().empty()) {
                          audit.diagnostics.push_back("project_localization_audit_bundle_locale_missing:" +
                                                      entry.path().filename().string());
                      }
                      if (!catalog.hasFontProfile()) {
                          audit.missing_font_profile_locales.push_back(locale);
                      }
                      const auto keys = catalog.getAllKeys();
                      available.insert(keys.begin(), keys.end());
                      keys_by_locale[locale].insert(keys.begin(), keys.end());
                  }, audit.diagnostics);

    const auto addReference = [&](const std::string& key, const std::filesystem::path& path,
                                  const std::string& ownerKind, const std::string& localId) {
        if (!key.empty()) audit.references.push_back({key, path, ownerKind, localId});
    };
    const auto stringField = [](const nlohmann::json& object, const char* field) {
        return object.contains(field) && object[field].is_string() ? object[field].get<std::string>() : std::string{};
    };
    scanDirectory(project_root / "content" / "dialogues", "project_localization_audit_dialogues_unreadable",
                  [&](const auto& entry) {
                      std::ifstream input(entry.path(), std::ios::binary);
                      const auto document = nlohmann::json::parse(input, nullptr, false);
                      if (document.is_discarded() || !document.is_object() || !document.contains("schema_version") ||
                          !document["schema_version"].is_string() ||
                          document["schema_version"].get<std::string>() != "urpg.dialogue_graph.v1" ||
                          !document.contains("nodes") || !document["nodes"].is_array()) {
                          audit.diagnostics.push_back("project_localization_audit_dialogue_invalid:" +
                                                      entry.path().filename().string());
                          return;
                      }
                      for (const auto& node : document["nodes"]) {
                          if (!node.is_object()) continue;
                          const auto id = stringField(node, "id");
                          addReference(stringField(node, "localization_key"), entry.path(), "dialogue.node", id);
                          addReference(stringField(node, "caption_localization_key"), entry.path(), "dialogue.caption", id);
                      }
                  }, audit.diagnostics);
    scanDirectory(project_root / "content" / "quests", "project_localization_audit_quests_unreadable",
                  [&](const auto& entry) {
                      std::ifstream input(entry.path(), std::ios::binary);
                      const auto document = nlohmann::json::parse(input, nullptr, false);
                      if (document.is_discarded() || !document.is_object() || !document.contains("schema_version") ||
                          !document["schema_version"].is_string() ||
                          document["schema_version"].get<std::string>() != "urpg.quest_objective_graph.v1" ||
                          !document.contains("nodes") || !document["nodes"].is_array()) {
                          audit.diagnostics.push_back("project_localization_audit_quest_invalid:" +
                                                      entry.path().filename().string());
                          return;
                      }
                      for (const auto& node : document["nodes"]) {
                          if (node.is_object()) {
                              addReference(stringField(node, "localization_key"), entry.path(), "quest.node",
                                           stringField(node, "id"));
                          }
                      }
                  }, audit.diagnostics);

    audit.available_keys.assign(available.begin(), available.end());
    std::set<std::string> referenced;
    for (const auto& reference : audit.references) referenced.insert(reference.key);
    for (const auto& key : referenced) {
        if (!available.contains(key)) audit.missing_referenced_keys.push_back(key);
    }
    for (const auto& [locale, keys] : keys_by_locale) {
        for (const auto& key : referenced) {
            if (!keys.contains(key)) audit.missing_referenced_locale_keys.push_back({locale, key});
        }
    }
    std::set_difference(available.begin(), available.end(), referenced.begin(), referenced.end(),
                        std::back_inserter(audit.unused_key_candidates));
    std::sort(audit.references.begin(), audit.references.end(), [](const auto& left, const auto& right) {
        return std::tie(left.key, left.document_path, left.owner_kind, left.local_id) <
               std::tie(right.key, right.document_path, right.owner_kind, right.local_id);
    });
    sortUnique(audit.missing_referenced_keys);
    sortUnique(audit.missing_font_profile_locales);
    std::sort(audit.missing_referenced_locale_keys.begin(), audit.missing_referenced_locale_keys.end(),
              [](const auto& left, const auto& right) {
                  return std::tie(left.locale, left.key) < std::tie(right.locale, right.key);
              });
    audit.missing_referenced_locale_keys.erase(
        std::unique(audit.missing_referenced_locale_keys.begin(), audit.missing_referenced_locale_keys.end(),
                    [](const auto& left, const auto& right) {
                        return left.locale == right.locale && left.key == right.key;
                    }),
        audit.missing_referenced_locale_keys.end());
    sortUnique(audit.unused_key_candidates);
    sortUnique(audit.diagnostics);
    return audit;
}

} // namespace urpg::localization
