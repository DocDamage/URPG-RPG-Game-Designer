#include "engine/core/assets/project_asset_reference_index.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <set>
#include <tuple>

namespace urpg::assets {

std::vector<ProjectAssetReference> ProjectAssetReferenceIndex::inboundForAsset(const std::string& asset_id) const {
    std::vector<ProjectAssetReference> result;
    for (const auto& reference : references) {
        if (reference.asset_id == asset_id) result.push_back(reference);
    }
    return result;
}

std::vector<ProjectAssetReference> ProjectAssetReferenceIndex::outboundForDocument(
    const std::filesystem::path& document_path) const {
    std::vector<ProjectAssetReference> result;
    const auto normalized = document_path.lexically_normal();
    for (const auto& reference : references) {
        if (reference.document_path.lexically_normal() == normalized) result.push_back(reference);
    }
    return result;
}

ProjectAssetReferenceIndex buildPerspective2DAssetReferenceIndex(const std::filesystem::path& project_root) {
    ProjectAssetReferenceIndex index;
    std::error_code error;
    const auto maps = project_root / "content" / "maps";
    for (const auto& entry : std::filesystem::directory_iterator(maps, error)) {
        if (error) {
            index.diagnostics.push_back("project_asset_reference_maps_unreadable");
            break;
        }
        if (!entry.is_regular_file() || entry.path().extension() != ".json" ||
            entry.path().filename().string().find(".p2d.json") == std::string::npos) {
            continue;
        }
        std::ifstream input(entry.path(), std::ios::binary);
        const auto document = nlohmann::json::parse(input, nullptr, false);
        if (document.is_discarded() || !document.is_object()) {
            index.diagnostics.push_back("project_asset_reference_p2d_document_invalid:" + entry.path().filename().string());
            continue;
        }
        const auto reference_count_before = index.references.size();
        try {
            const auto add = [&](const std::string& asset_id, const std::string& owner_kind,
                                 const std::string& local_id, const std::string& project_path = {}) {
                if (!asset_id.empty()) index.references.push_back({asset_id, owner_kind, entry.path(), local_id, project_path});
            };
            for (const auto& option : document.value("tile_palette", nlohmann::json::array())) {
                if (option.is_object()) add(option.value("asset_id", ""), "perspective_2d.tile_palette",
                                            option.value("option_id", ""), option.value("project_path", ""));
            }
            for (const auto& option : document.value("prop_palette", nlohmann::json::array())) {
                if (option.is_object()) add(option.value("asset_id", ""), "perspective_2d.prop_palette",
                                            option.value("asset_id", ""), option.value("project_path", ""));
            }
            for (const auto& prop : document.value("props", nlohmann::json::array())) {
                if (prop.is_object()) add(prop.value("asset_id", ""), "perspective_2d.prop",
                                          prop.value("instance_id", ""));
            }
            for (const auto& event : document.value("events", nlohmann::json::array())) {
                if (event.is_object()) add(event.value("asset_id", ""), "perspective_2d.event",
                                           event.value("event_id", ""), event.value("asset_project_path", ""));
            }
        } catch (const nlohmann::json::exception&) {
            index.references.resize(reference_count_before);
            index.diagnostics.push_back("project_asset_reference_p2d_document_invalid:" + entry.path().filename().string());
        }
    }
    std::sort(index.references.begin(), index.references.end(), [](const auto& left, const auto& right) {
        return std::tie(left.asset_id, left.document_path, left.owner_kind, left.local_id) <
               std::tie(right.asset_id, right.document_path, right.owner_kind, right.local_id);
    });
    return index;
}

ProjectAssetReferenceIndex buildProjectAssetReferenceIndex(const std::filesystem::path& project_root) {
    auto index = buildPerspective2DAssetReferenceIndex(project_root);
    std::error_code error;
    const auto dialogues = project_root / "content" / "dialogues";
    for (const auto& entry : std::filesystem::directory_iterator(dialogues, error)) {
        if (error) {
            index.diagnostics.push_back("project_asset_reference_dialogues_unreadable");
            break;
        }
        if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;
        std::ifstream input(entry.path(), std::ios::binary);
        const auto document = nlohmann::json::parse(input, nullptr, false);
        if (document.is_discarded() || !document.is_object() || !document.contains("schema_version") ||
            !document["schema_version"].is_string() || document["schema_version"].get<std::string>() != "urpg.dialogue_graph.v1" ||
            !document.contains("nodes") || !document["nodes"].is_array()) {
            index.diagnostics.push_back("project_asset_reference_dialogue_document_invalid:" + entry.path().filename().string());
            continue;
        }
        const auto reference_count_before = index.references.size();
        try {
            for (const auto& node : document["nodes"]) {
                if (!node.is_object() || !node.contains("voice_asset_id") || !node["voice_asset_id"].is_string()) continue;
                const auto asset_id = node["voice_asset_id"].get<std::string>();
                if (!asset_id.empty()) {
                    index.references.push_back({asset_id, "dialogue.voice", entry.path(), node.value("id", ""), {}});
                }
            }
        } catch (const nlohmann::json::exception&) {
            index.references.resize(reference_count_before);
            index.diagnostics.push_back("project_asset_reference_dialogue_document_invalid:" + entry.path().filename().string());
        }
    }
    error.clear();
    const auto characters = project_root / "content" / "characters";
    for (const auto& entry : std::filesystem::directory_iterator(characters, error)) {
        if (error) {
            index.diagnostics.push_back("project_asset_reference_characters_unreadable");
            break;
        }
        if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;
        std::ifstream input(entry.path(), std::ios::binary);
        const auto document = nlohmann::json::parse(input, nullptr, false);
        if (document.is_discarded() || !document.is_object() || !document.contains("schemaVersion") ||
            !document["schemaVersion"].is_string() || document["schemaVersion"].get<std::string>() != "1.0.0") {
            index.diagnostics.push_back("project_asset_reference_character_document_invalid:" + entry.path().filename().string());
            continue;
        }
        const auto reference_count_before = index.references.size();
        try {
            const auto add = [&](const char* field, const char* owner_kind) {
                const auto asset_id = document.value(field, std::string{});
                if (!asset_id.empty()) index.references.push_back({asset_id, owner_kind, entry.path(), field, {}});
            };
            add("portraitAssetId", "character.portrait");
            add("fieldSpriteAssetId", "character.field_sprite");
            add("battleSpriteAssetId", "character.battle_sprite");
            for (const auto& asset_id : document.value("layeredPartAssetIds", nlohmann::json::array())) {
                if (asset_id.is_string() && !asset_id.get<std::string>().empty()) {
                    index.references.push_back({asset_id.get<std::string>(), "character.layered_part", entry.path(),
                                                asset_id.get<std::string>(), {}});
                }
            }
        } catch (const nlohmann::json::exception&) {
            index.references.resize(reference_count_before);
            index.diagnostics.push_back("project_asset_reference_character_document_invalid:" + entry.path().filename().string());
        }
    }
    error.clear();
    const auto part_catalogs = project_root / "content" / "part_catalogs";
    for (const auto& entry : std::filesystem::directory_iterator(part_catalogs, error)) {
        if (error) {
            index.diagnostics.push_back("project_asset_reference_part_catalogs_unreadable");
            break;
        }
        if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;
        std::ifstream input(entry.path(), std::ios::binary);
        const auto document = nlohmann::json::parse(input, nullptr, false);
        if (document.is_discarded() || !document.is_object() || !document.contains("schemaVersion") ||
            !document["schemaVersion"].is_number_integer() || document["schemaVersion"].get<int>() != 1 ||
            !document.contains("parts") || !document["parts"].is_array()) {
            index.diagnostics.push_back("project_asset_reference_part_catalog_invalid:" + entry.path().filename().string());
            continue;
        }
        const auto reference_count_before = index.references.size();
        try {
            for (const auto& part : document["parts"]) {
                if (!part.is_object() || !part.contains("partId") || !part["partId"].is_string() ||
                    !part.contains("assetId") || !part["assetId"].is_string()) {
                    continue;
                }
                const auto asset_id = part["assetId"].get<std::string>();
                if (!asset_id.empty()) {
                    index.references.push_back(
                        {asset_id, "grid_part_catalog.part", entry.path(), part["partId"].get<std::string>(), {}});
                }
            }
        } catch (const nlohmann::json::exception&) {
            index.references.resize(reference_count_before);
            index.diagnostics.push_back("project_asset_reference_part_catalog_invalid:" + entry.path().filename().string());
        }
    }
    const auto audio_mix_path = project_root / "config" / "audio_mix_presets.json";
    if (std::ifstream input(audio_mix_path, std::ios::binary); input.good()) {
        const auto document = nlohmann::json::parse(input, nullptr, false);
        try {
            if (document.is_discarded() || !document.is_object() ||
                document.value("schema", "") != "urpg.project_audio_mix.v1") {
                index.diagnostics.push_back("project_asset_reference_audio_mix_invalid");
            } else {
                const auto asset_id = document.value("encounter_preview_asset_id", std::string{});
                if (!asset_id.empty()) {
                    index.references.push_back({asset_id, "audio_mix.encounter_preview", audio_mix_path,
                                                "encounter_preview_asset_id", {}});
                }
            }
        } catch (const nlohmann::json::exception&) {
            index.diagnostics.push_back("project_asset_reference_audio_mix_invalid");
        }
    }
    std::sort(index.references.begin(), index.references.end(), [](const auto& left, const auto& right) {
        return std::tie(left.asset_id, left.document_path, left.owner_kind, left.local_id) <
               std::tie(right.asset_id, right.document_path, right.owner_kind, right.local_id);
    });
    return index;
}

ProjectAssetRemovalImpactPlan buildProjectAssetRemovalImpactPlan(const std::filesystem::path& project_root,
                                                                  const std::string& asset_id) {
    ProjectAssetRemovalImpactPlan plan;
    plan.asset_id = asset_id;
    if (asset_id.empty()) {
        plan.diagnostics.push_back("project_asset_removal_impact_asset_id_missing");
        return plan;
    }
    if (project_root.empty()) {
        plan.diagnostics.push_back("project_asset_removal_impact_project_root_missing");
        return plan;
    }
    const auto index = buildProjectAssetReferenceIndex(project_root);
    plan.inbound_references = index.inboundForAsset(asset_id);
    plan.diagnostics = index.diagnostics;
    plan.is_orphan_candidate = plan.inbound_references.empty();
    return plan;
}

std::vector<std::string> findOrphanedAttachedAssetIds(const std::filesystem::path& project_root) {
    const auto index = buildProjectAssetReferenceIndex(project_root);
    std::set<std::string> referenced;
    for (const auto& reference : index.references) referenced.insert(reference.asset_id);
    std::set<std::string> attached;
    std::error_code error;
    const auto manifests = project_root / "content" / "assets" / "manifests";
    for (const auto& entry : std::filesystem::directory_iterator(manifests, error)) {
        if (error || !entry.is_regular_file() || entry.path().extension() != ".json") continue;
        std::ifstream input(entry.path(), std::ios::binary);
        const auto manifest = nlohmann::json::parse(input, nullptr, false);
        if (!manifest.is_object() || !manifest.contains("assetId") || !manifest["assetId"].is_string()) continue;
        const auto asset_id = manifest["assetId"].get<std::string>();
        if (!asset_id.empty()) attached.insert(asset_id);
    }
    std::vector<std::string> orphaned;
    std::set_difference(attached.begin(), attached.end(), referenced.begin(), referenced.end(),
                        std::back_inserter(orphaned));
    return orphaned;
}

} // namespace urpg::assets
