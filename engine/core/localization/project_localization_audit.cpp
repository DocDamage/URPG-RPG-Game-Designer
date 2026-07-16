#include "engine/core/localization/project_localization_audit.h"

#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/localization/locale_catalog.h"

#include <algorithm>
#include <cctype>
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

bool pathInside(const std::filesystem::path& root, const std::filesystem::path& candidate) {
    const auto relative = candidate.lexically_relative(root);
    return !relative.empty() && !relative.is_absolute() && relative.begin()->string() != "..";
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

bool isSafeOpaqueId(const std::string& value) {
    const auto path = std::filesystem::path(value);
    return !value.empty() && value != "." && value != ".." && !path.has_parent_path() && !path.has_root_path() &&
           std::all_of(value.begin(), value.end(), [](const unsigned char character) {
               return std::isalnum(character) != 0 || character == '-' || character == '_' || character == '.';
           });
}

bool isLocaleTag(const std::string& value) {
    if (value.empty() || value.size() > 64) return false;
    size_t segmentStart = 0;
    size_t segmentIndex = 0;
    while (segmentStart < value.size()) {
        const auto segmentEnd = value.find('-', segmentStart);
        const auto length = (segmentEnd == std::string::npos ? value.size() : segmentEnd) - segmentStart;
        if (length == 0 || length > 8 || (segmentIndex == 0 && length < 2)) return false;
        for (size_t index = segmentStart; index < segmentStart + length; ++index) {
            const auto character = static_cast<unsigned char>(value[index]);
            if ((segmentIndex == 0 && std::isalpha(character) == 0) ||
                (segmentIndex != 0 && std::isalnum(character) == 0)) {
                return false;
            }
        }
        if (segmentEnd == std::string::npos) return true;
        segmentStart = segmentEnd + 1;
        ++segmentIndex;
    }
    return false;
}

struct VoiceTakeMetadata {
    std::string locale;
    std::string takeId;
    std::string mutedAlternativeAssetId;
    bool present = false;
    bool valid = false;
};

VoiceTakeMetadata inspectVoiceTakeMetadata(const assets::AssetPromotionManifest& manifest) {
    VoiceTakeMetadata metadata;
    const auto voiceTake = manifest.authoredMetadata.find("voice_take");
    if (voiceTake == manifest.authoredMetadata.end()) return metadata;
    metadata.present = true;
    if (!voiceTake->is_object()) return metadata;

    const auto readString = [&](const char* field) {
        const auto value = voiceTake->find(field);
        return value != voiceTake->end() && value->is_string() ? value->get<std::string>() : std::string{};
    };
    const auto schema = readString("schema");
    metadata.locale = readString("locale");
    metadata.takeId = readString("take_id");
    metadata.mutedAlternativeAssetId = readString("muted_alternative_asset_id");
    metadata.valid = schema == "urpg.promoted_audio_voice_take.v1" && isLocaleTag(metadata.locale) &&
                     isSafeOpaqueId(metadata.takeId) &&
                     (metadata.mutedAlternativeAssetId.empty() ||
                      (isSafeOpaqueId(metadata.mutedAlternativeAssetId) &&
                       metadata.mutedAlternativeAssetId != manifest.assetId));
    return metadata;
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
    std::map<std::string, std::string> profile_by_locale;
    std::map<std::string, assets::AssetPromotionManifest> attached_audio_assets;
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
                      } else {
                          profile_by_locale.insert_or_assign(locale, catalog.getFontProfileId());
                      }
                      const auto keys = catalog.getAllKeys();
                      available.insert(keys.begin(), keys.end());
                      keys_by_locale[locale].insert(keys.begin(), keys.end());
                  }, audit.diagnostics);

    const auto fontProfiles = FontProfileRegistry::loadProjectManifest(project_root);
    audit.font_profile_diagnostics = fontProfiles.diagnostics;
    for (const auto& [locale, profile] : profile_by_locale) {
        if (!fontProfiles.registry || !fontProfiles.registry->hasProfile(profile)) {
            audit.unresolved_font_profile_locales.push_back(locale);
        }
    }

    std::error_code contentError;
    const auto contentRoot = std::filesystem::weakly_canonical(project_root / "content", contentError);
    scanDirectory(project_root / "content" / "assets" / "manifests",
                  "project_localization_audit_asset_manifests_unreadable", [&](const auto& entry) {
                      std::ifstream input(entry.path(), std::ios::binary);
                      const auto json = nlohmann::json::parse(input, nullptr, false);
                      if (json.is_discarded()) {
                          audit.diagnostics.push_back("project_localization_audit_asset_manifest_invalid:" +
                                                      entry.path().filename().string());
                          return;
                      }
                      try {
                          const auto manifest = assets::deserializeAssetPromotionManifest(json);
                          if (manifest.preview.kind != "audio") return;
                          std::error_code payloadError;
                          const auto payload = std::filesystem::weakly_canonical(manifest.promotedPath, payloadError);
                          if (contentError || payloadError || manifest.assetId.empty() ||
                              manifest.status != assets::AssetPromotionStatus::RuntimeReady ||
                              !manifest.package.includeInRuntime || !std::filesystem::is_regular_file(payload) ||
                              !pathInside(contentRoot, payload)) {
                              audit.diagnostics.push_back("project_localization_audit_audio_asset_invalid:" +
                                                          entry.path().filename().string());
                              return;
                          }
                          attached_audio_assets.insert_or_assign(manifest.assetId, manifest);
                      } catch (const nlohmann::json::exception&) {
                          audit.diagnostics.push_back("project_localization_audit_asset_manifest_invalid:" +
                                                      entry.path().filename().string());
                      }
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
                          const auto voiceAssetId = stringField(node, "voice_asset_id");
                          const auto captionKey = stringField(node, "caption_localization_key");
                          if (!voiceAssetId.empty() || !captionKey.empty()) {
                              audit.dialogue_media_references.push_back(
                                  {entry.path(), id, voiceAssetId, captionKey});
                          }
                          if (node.contains("choices") && node["choices"].is_array()) {
                              for (const auto& choice : node["choices"]) {
                                  if (!choice.is_object()) continue;
                                  const auto choice_id = stringField(choice, "id");
                                  addReference(stringField(choice, "localization_key"), entry.path(),
                                               "dialogue.choice", id + ":" + choice_id);
                              }
                          }
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
    for (auto& reference : audit.dialogue_media_references) {
        const auto attachedVoice = attached_audio_assets.find(reference.voice_asset_id);
        reference.voice_asset_attached = !reference.voice_asset_id.empty() && attachedVoice != attached_audio_assets.end();
        reference.caption_key_available = !reference.caption_key.empty() && available.contains(reference.caption_key);
        if (!reference.voice_asset_id.empty() && !reference.voice_asset_attached) {
            audit.dialogue_media_issues.push_back(
                {"dialogue_voice_asset_missing", reference.document_path, reference.node_id, reference.voice_asset_id,
                 reference.caption_key});
        }
        if (!reference.voice_asset_id.empty() && reference.caption_key.empty()) {
            audit.dialogue_media_issues.push_back(
                {"dialogue_voice_caption_missing", reference.document_path, reference.node_id, reference.voice_asset_id,
                 {}});
        }
        if (reference.voice_asset_id.empty() && !reference.caption_key.empty()) {
            audit.dialogue_media_issues.push_back(
                {"dialogue_caption_without_voice", reference.document_path, reference.node_id, {}, reference.caption_key});
        }
        if (!reference.caption_key.empty() && !reference.caption_key_available) {
            audit.dialogue_media_issues.push_back(
                {"dialogue_voice_caption_key_missing", reference.document_path, reference.node_id, reference.voice_asset_id,
                 reference.caption_key});
        }
        if (attachedVoice != attached_audio_assets.end()) {
            const auto metadata = inspectVoiceTakeMetadata(attachedVoice->second);
            reference.voice_take_metadata_present = metadata.present;
            reference.voice_take_metadata_valid = metadata.valid;
            reference.voice_take_locale = metadata.locale;
            reference.voice_take_id = metadata.takeId;
            reference.muted_alternative_asset_id = metadata.mutedAlternativeAssetId;
            if (!metadata.present) {
                audit.dialogue_media_issues.push_back(
                    {"dialogue_voice_take_metadata_missing", reference.document_path, reference.node_id,
                     reference.voice_asset_id, reference.caption_key});
            } else if (!metadata.valid) {
                audit.dialogue_media_issues.push_back(
                    {"dialogue_voice_take_metadata_invalid", reference.document_path, reference.node_id,
                     reference.voice_asset_id, reference.caption_key});
            } else if (!metadata.mutedAlternativeAssetId.empty()) {
                reference.muted_alternative_asset_attached =
                    attached_audio_assets.contains(metadata.mutedAlternativeAssetId);
                if (!reference.muted_alternative_asset_attached) {
                    audit.dialogue_media_issues.push_back(
                        {"dialogue_voice_take_muted_alternative_attachment_invalid", reference.document_path,
                         reference.node_id, reference.voice_asset_id, reference.caption_key});
                }
            }
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
    sortUnique(audit.unresolved_font_profile_locales);
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
    std::sort(audit.dialogue_media_references.begin(), audit.dialogue_media_references.end(),
              [](const auto& left, const auto& right) {
                  return std::tie(left.document_path, left.node_id, left.voice_asset_id, left.caption_key) <
                         std::tie(right.document_path, right.node_id, right.voice_asset_id, right.caption_key);
              });
    std::sort(audit.dialogue_media_issues.begin(), audit.dialogue_media_issues.end(),
              [](const auto& left, const auto& right) {
                  return std::tie(left.code, left.document_path, left.node_id, left.voice_asset_id, left.caption_key) <
                         std::tie(right.code, right.document_path, right.node_id, right.voice_asset_id, right.caption_key);
              });
    sortUnique(audit.diagnostics);
    return audit;
}

} // namespace urpg::localization
