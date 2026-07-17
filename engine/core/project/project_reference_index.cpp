#include "engine/core/project/project_reference_index.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <tuple>

#include <nlohmann/json.hpp>

namespace urpg::project {
namespace {

std::string normalizedSearchText(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

std::set<std::string> searchGrams(const std::string& text) {
    std::set<std::string> grams;
    for (size_t length = 1; length <= 3; ++length) {
        if (text.size() < length) break;
        for (size_t offset = 0; offset + length <= text.size(); ++offset) {
            grams.insert(text.substr(offset, length));
        }
    }
    return grams;
}

auto edgeKey(const ProjectReferenceEdge& edge) {
    return std::tie(edge.document_path, edge.source_type, edge.source_id, edge.reference_type, edge.target_type,
                    edge.target_id, edge.local_id, edge.package_inclusion);
}

ProjectReferenceUpdateResult normalizeDocument(const ProjectReferenceDocument& document,
                                                std::vector<ProjectReferenceEdge>& normalized) {
    if (document.document_path.empty()) {
        return {false, "project_reference_document_path_missing", "Reference documents require a project path."};
    }
    const auto path = document.document_path.lexically_normal();
    normalized = document.edges;
    for (auto& edge : normalized) {
        if (edge.source_type.empty() || edge.source_id.empty() || edge.target_type.empty() || edge.target_id.empty() ||
            edge.reference_type.empty()) {
            return {false, "project_reference_edge_invalid", "Reference edges require stable source, target, and type IDs."};
        }
        edge.document_path = path;
    }
    std::sort(normalized.begin(), normalized.end(), [](const auto& left, const auto& right) {
        return edgeKey(left) < edgeKey(right);
    });
    if (std::adjacent_find(normalized.begin(), normalized.end()) != normalized.end()) {
        return {false, "project_reference_edge_duplicate", "A document emitted a duplicate stable reference edge."};
    }
    return {true, "project_reference_document_valid", "Reference document edges are valid."};
}

void addEdge(ProjectReferenceDocument& document, std::string sourceType, std::string sourceId,
             std::string targetType, std::string targetId, std::string referenceType, std::string localId,
             const bool packageInclusion = false) {
    if (targetId.empty()) return;
    document.edges.push_back({std::move(sourceType), std::move(sourceId), std::move(targetType), std::move(targetId),
                              std::move(referenceType), document.document_path, std::move(localId), packageInclusion});
}

std::string argumentKey(const std::string& argument) {
    const auto end = argument.find_first_of("=+:-");
    return argument.substr(0, end);
}

void extractEventCommands(ProjectReferenceDocument& result, const nlohmann::json& commands,
                          const std::string& eventSourceId, const std::string& localPrefix) {
    if (!commands.is_array()) return;
    for (size_t index = 0; index < commands.size(); ++index) {
        const auto& command = commands[index];
        if (!command.is_object()) continue;
        const auto code = command.value("code", "");
        const auto argument = command.value("argument", "");
        const auto local = localPrefix + ".command:" + std::to_string(index);
        if (code == "change_switch") addEdge(result, "event", eventSourceId, "switch", argumentKey(argument), code, local);
        else if (code == "change_variable") addEdge(result, "event", eventSourceId, "variable", argumentKey(argument), code, local);
        else if (code == "change_self_switch") addEdge(result, "event", eventSourceId, "self_switch", argumentKey(argument), code, local);
        else if (code == "change_item") addEdge(result, "event", eventSourceId, "item", argumentKey(argument), code, local);
        else if (code == "call_common_event") addEdge(result, "event", eventSourceId, "common_event", argument, code, local);
        else if (code == "start_battle") addEdge(result, "event", eventSourceId, "encounter", argument, code, local);
        else if (code == "open_vendor") addEdge(result, "event", eventSourceId, "vendor", argument, code, local);
        else if (code == "start_dialogue") addEdge(result, "event", eventSourceId, "dialogue", argument, code, local);
        else if (code == "transfer_player") {
            addEdge(result, "event", eventSourceId, "map", argument.substr(0, argument.find(':')), code, local);
        } else if (code == "conditional_branch") {
            const auto condition = command.value("condition", nlohmann::json::object());
            const auto type = condition.value("type", "");
            addEdge(result, "event", eventSourceId, type, condition.value("key", ""), "branch_condition", local);
            extractEventCommands(result, command.value("true_commands", nlohmann::json::array()), eventSourceId,
                                 local + ".true");
            extractEventCommands(result, command.value("false_commands", nlohmann::json::array()), eventSourceId,
                                 local + ".false");
        }
    }
}

} // namespace

ProjectReferenceUpdateResult ProjectReferenceIndex::rebuild(const std::vector<ProjectReferenceDocument>& documents) {
    ProjectReferenceIndex candidate;
    for (const auto& document : documents) {
        const auto result = candidate.replaceDocument(document);
        if (!result.success) return result;
    }
    edges_ = std::move(candidate.edges_);
    rebuildQueryIndexes();
    return {true, "project_reference_index_rebuilt", "Project reference index rebuilt deterministically."};
}

ProjectReferenceUpdateResult ProjectReferenceIndex::replaceDocument(const ProjectReferenceDocument& document) {
    std::vector<ProjectReferenceEdge> replacement;
    const auto validation = normalizeDocument(document, replacement);
    if (!validation.success) return validation;
    const auto path = document.document_path.lexically_normal();
    auto candidate = edges_;
    std::erase_if(candidate, [&](const auto& edge) { return edge.document_path.lexically_normal() == path; });
    candidate.insert(candidate.end(), replacement.begin(), replacement.end());
    std::sort(candidate.begin(), candidate.end(), [](const auto& left, const auto& right) {
        return edgeKey(left) < edgeKey(right);
    });
    edges_ = std::move(candidate);
    rebuildQueryIndexes();
    return {true, "project_reference_document_replaced", "Document reference edges replaced atomically."};
}

void ProjectReferenceIndex::removeDocument(const std::filesystem::path& document_path) {
    const auto path = document_path.lexically_normal();
    const auto previousSize = edges_.size();
    std::erase_if(edges_, [&](const auto& edge) { return edge.document_path.lexically_normal() == path; });
    if (edges_.size() != previousSize) rebuildQueryIndexes();
}

std::vector<ProjectReferenceEdge> ProjectReferenceIndex::inbound(const std::string_view target_type,
                                                                 const std::string_view target_id) const {
    return queryPosting(inbound_index_, target_type, target_id);
}

std::vector<ProjectReferenceEdge> ProjectReferenceIndex::outbound(const std::string_view source_type,
                                                                  const std::string_view source_id) const {
    return queryPosting(outbound_index_, source_type, source_id);
}

std::vector<ProjectReferenceEdge> ProjectReferenceIndex::whyIncluded(const std::string_view target_type,
                                                                     const std::string_view target_id) const {
    return queryPosting(package_inclusion_index_, target_type, target_id);
}

ProjectReferenceQueryResult ProjectReferenceIndex::findUses(const std::string_view target_type,
                                                            const std::string_view target_id) const {
    ProjectReferenceQueryResult result;
    result.object_type = target_type;
    result.object_id = target_id;
    if (target_type.empty() || target_id.empty()) {
        last_query_candidate_count_ = 0;
        result.code = "project_reference_query_object_missing";
        return result;
    }
    result.matches = inbound(target_type, target_id);
    result.success = true;
    result.code = result.matches.empty() ? "project_reference_query_no_uses" : "project_reference_query_uses_found";
    return result;
}

ProjectReferenceQueryResult ProjectReferenceIndex::findReferences(const std::string_view source_type,
                                                                  const std::string_view source_id) const {
    ProjectReferenceQueryResult result;
    result.object_type = source_type;
    result.object_id = source_id;
    if (source_type.empty() || source_id.empty()) {
        last_query_candidate_count_ = 0;
        result.code = "project_reference_query_object_missing";
        return result;
    }
    result.matches = outbound(source_type, source_id);
    result.success = true;
    result.code = result.matches.empty() ? "project_reference_query_no_references"
                                         : "project_reference_query_references_found";
    return result;
}

ProjectReferenceQueryResult ProjectReferenceIndex::explainInclusion(const std::string_view target_type,
                                                                    const std::string_view target_id) const {
    ProjectReferenceQueryResult result;
    result.object_type = target_type;
    result.object_id = target_id;
    if (target_type.empty() || target_id.empty()) {
        last_query_candidate_count_ = 0;
        result.code = "project_reference_query_object_missing";
        return result;
    }
    result.matches = whyIncluded(target_type, target_id);
    result.success = true;
    result.code = result.matches.empty() ? "project_reference_query_not_package_included"
                                         : "project_reference_query_inclusion_explained";
    return result;
}

std::vector<ProjectReferenceObjectSearchResult> ProjectReferenceIndex::searchObjects(
    const std::string_view query, const size_t limit) const {
    std::vector<ProjectReferenceObjectSearchResult> result;
    if (limit == 0) {
        last_query_candidate_count_ = 0;
        return result;
    }
    const auto needle = normalizedSearchText(query);
    std::vector<size_t> candidates;
    if (needle.empty()) {
        candidates.resize(searchable_objects_.size());
        for (size_t index = 0; index < candidates.size(); ++index) candidates[index] = index;
    } else {
        const auto gramLength = std::min<size_t>(3, needle.size());
        bool first = true;
        for (size_t offset = 0; offset + gramLength <= needle.size(); ++offset) {
            const auto posting = object_search_postings_.find(needle.substr(offset, gramLength));
            if (posting == object_search_postings_.end()) {
                last_query_candidate_count_ = 0;
                return result;
            }
            if (first) {
                candidates = posting->second;
                first = false;
            } else {
                std::vector<size_t> intersection;
                std::set_intersection(candidates.begin(), candidates.end(), posting->second.begin(),
                                      posting->second.end(), std::back_inserter(intersection));
                candidates = std::move(intersection);
            }
            if (candidates.empty()) break;
        }
    }
    last_query_candidate_count_ = candidates.size();
    for (const auto index : candidates) {
        const auto& object = searchable_objects_[index];
        const auto text = normalizedSearchText(object.object_type + " " + object.object_id);
        if (!needle.empty() && text.find(needle) == std::string::npos) continue;
        result.push_back(object);
        if (result.size() == limit) break;
    }
    return result;
}

ProjectReferenceNavigationTarget ProjectReferenceIndex::navigationTarget(
    const std::string_view object_type, const std::string_view object_id) const {
    ProjectReferenceNavigationTarget result;
    result.object_type = object_type;
    result.object_id = object_id;
    if (object_type.empty() || object_id.empty()) {
        result.code = "project_reference_navigation_object_missing";
        return result;
    }
    const auto candidates = searchObjects(object_id, 100);
    const auto object = std::find_if(candidates.begin(), candidates.end(), [&](const auto& candidate) {
        return candidate.object_type == object_type && candidate.object_id == object_id;
    });
    if (object == candidates.end()) {
        result.code = "project_reference_navigation_object_not_indexed";
        return result;
    }
    result.document_path = object->document_path;
    result.local_id = object->local_id;
    const auto type = std::string(object_type);
    if (type == "asset") result.panel_id = "assets";
    else if (type == "plugin" || type == "plugin_package" || type == "mod" || type == "script") {
        result.panel_id = "mod";
    } else if (type == "ability" || type == "effect" || type == "gameplay_feature" || type == "recipe") {
        result.panel_id = "ability";
    } else if (type == "character") {
        result.panel_id = "character_creator";
    } else if (type == "map" || type == "grid_part_instance" || type == "grid_part" || type == "event" ||
               type == "dialogue" || type == "dialogue_node" || type == "quest" || type == "quest_node") {
        result.panel_id = "spatial_authoring";
    } else {
        result.panel_id = "diagnostics";
    }
    result.success = true;
    result.code = "project_reference_navigation_ready";
    return result;
}

void ProjectReferenceIndex::rebuildQueryIndexes() {
    inbound_index_.clear();
    outbound_index_.clear();
    package_inclusion_index_.clear();
    searchable_objects_.clear();
    object_search_postings_.clear();
    std::map<ObjectKey, ProjectReferenceObjectSearchResult> objects;
    for (size_t index = 0; index < edges_.size(); ++index) {
        const auto& edge = edges_[index];
        inbound_index_[{edge.target_type, edge.target_id}].push_back(index);
        outbound_index_[{edge.source_type, edge.source_id}].push_back(index);
        if (edge.package_inclusion) {
            package_inclusion_index_[{edge.target_type, edge.target_id}].push_back(index);
        }
        const auto addObject = [&](const std::string& type, const std::string& id) -> ProjectReferenceObjectSearchResult& {
            auto [found, inserted] = objects.try_emplace({type, id});
            if (inserted) {
                found->second.object_type = type;
                found->second.object_id = id;
                found->second.document_path = edge.document_path;
                found->second.local_id = edge.local_id;
            } else if (!edge.document_path.empty() &&
                       (found->second.document_path.empty() || edge.document_path < found->second.document_path)) {
                found->second.document_path = edge.document_path;
                found->second.local_id = edge.local_id;
            }
            return found->second;
        };
        (void)addObject(edge.source_type, edge.source_id);
        auto& target = addObject(edge.target_type, edge.target_id);
        target.package_included = target.package_included || edge.package_inclusion;
    }
    searchable_objects_.reserve(objects.size());
    for (auto& [key, object] : objects) {
        object.inbound_count = inbound_index_[key].size();
        object.outbound_count = outbound_index_[key].size();
        searchable_objects_.push_back(std::move(object));
    }
    for (size_t index = 0; index < searchable_objects_.size(); ++index) {
        const auto& object = searchable_objects_[index];
        const auto text = normalizedSearchText(object.object_type + " " + object.object_id);
        for (const auto& gram : searchGrams(text)) object_search_postings_[gram].push_back(index);
    }
    last_query_candidate_count_ = 0;
    ++index_revision_;
}

std::vector<ProjectReferenceEdge>
ProjectReferenceIndex::queryPosting(const std::map<ObjectKey, std::vector<size_t>>& index,
                                    const std::string_view object_type, const std::string_view object_id) const {
    const auto found = index.find({std::string(object_type), std::string(object_id)});
    if (found == index.end()) {
        last_query_candidate_count_ = 0;
        return {};
    }
    last_query_candidate_count_ = found->second.size();
    std::vector<ProjectReferenceEdge> result;
    result.reserve(found->second.size());
    for (const auto edgeIndex : found->second) result.push_back(edges_[edgeIndex]);
    return result;
}

ProjectReferenceExtractionResult extractPerspective2DReferences(const std::filesystem::path& document_path,
                                                                const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("document_kind", "") != "urpg.perspective_2d.map" ||
        document.value("version", 0) != 1 || document.value("map_id", "").empty()) {
        result.code = "project_reference_perspective_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        const auto mapId = document["map_id"].get<std::string>();
        const auto addAssets = [&](const char* field, const char* localField, const char* referenceType) {
            for (const auto& row : document.value(field, nlohmann::json::array())) {
                if (!row.is_object()) continue;
                addEdge(result.document, "map", mapId, "asset", row.value("asset_id", ""), referenceType,
                        std::string(field) + ":" + row.value(localField, ""), true);
            }
        };
        addAssets("tileset_pages", "page_id", "tileset_page_asset");
        addAssets("tile_palette", "option_id", "tile_palette_asset");
        addAssets("prop_palette", "asset_id", "prop_palette_asset");
        for (const auto& prop : document.value("props", nlohmann::json::array())) {
            if (prop.is_object()) addEdge(result.document, "map", mapId, "asset", prop.value("asset_id", ""),
                                          "prop_asset", "prop:" + prop.value("instance_id", ""), true);
        }
        const auto projectDatabase = document.value("project_database", nlohmann::json::object());
        for (const auto& [field, type] : std::vector<std::pair<std::string, std::string>>{
                 {"actors", "actor"}, {"items", "item"}, {"switches", "switch"}, {"variables", "variable"},
                 {"common_events", "common_event"}, {"maps", "map"}, {"encounters", "encounter"},
                 {"assets", "asset"}}) {
            for (const auto& row : projectDatabase.value(field, nlohmann::json::array())) {
                if (row.is_object()) addEdge(result.document, "map", mapId, type, row.value("id", ""),
                                              "project_database_" + type, field + ":" + row.value("id", ""),
                                              type == "asset");
            }
        }
        for (const auto& event : document.value("events", nlohmann::json::array())) {
            if (!event.is_object()) continue;
            const auto eventId = event.value("event_id", "");
            if (eventId.empty()) continue;
            const auto sourceId = mapId + "/" + eventId;
            addEdge(result.document, "event", sourceId, "map", mapId, "owned_by_map", "event:" + eventId);
            addEdge(result.document, "event", sourceId, "asset", event.value("asset_id", ""), "event_sprite_asset",
                    "event:" + eventId, true);
            extractEventCommands(result.document, event.value("commands", nlohmann::json::array()), sourceId,
                                 "event:" + eventId);
            for (const auto& page : event.value("pages", nlohmann::json::array())) {
                if (!page.is_object()) continue;
                const auto pageId = page.value("page_id", "");
                for (const auto& condition : page.value("conditions", nlohmann::json::array())) {
                    if (condition.is_object()) addEdge(result.document, "event", sourceId, condition.value("type", ""),
                                                       condition.value("key", ""), "page_condition",
                                                       "event:" + eventId + ".page:" + pageId);
                }
                extractEventCommands(result.document, page.value("commands", nlohmann::json::array()), sourceId,
                                     "event:" + eventId + ".page:" + pageId);
            }
        }
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_perspective_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_perspective_document_extracted";
    return result;
}

ProjectReferenceExtractionResult extractGridPartReferences(const std::filesystem::path& document_path,
                                                           const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path;
    if (!document.is_object()) {
        result.code = "project_reference_grid_part_invalid";
        return result;
    }
    const auto mapId = document.value("mapId", "");
    if (mapId.empty() || !document.value("parts", nlohmann::json::array()).is_array()) {
        result.code = "project_reference_grid_part_invalid";
        return result;
    }
    for (const auto& part : document["parts"]) {
        if (!part.is_object()) continue;
        const auto instanceId = part.value("instanceId", "");
        const auto sourceId = mapId + "/" + instanceId;
        addEdge(result.document, "grid_part_instance", sourceId, "grid_part", part.value("partId", ""),
                "placed_part", "part:" + instanceId, true);
        const auto properties = part.value("properties", nlohmann::json::object());
        if (!properties.is_object()) continue;
        for (const auto& [key, value] : properties.items()) {
            if (value.is_string() && (key.ends_with("asset_id") || key.ends_with("assetId"))) {
                addEdge(result.document, "grid_part_instance", sourceId, "asset", value.get<std::string>(),
                        "part_property_asset", "part:" + instanceId + ".property:" + key, true);
            }
        }
    }
    result.success = true;
    result.code = "project_reference_grid_part_extracted";
    return result;
}

ProjectReferenceExtractionResult extractAbilityReferences(const std::filesystem::path& document_path,
                                                          const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path;
    if (!document.is_object()) {
        result.code = "project_reference_ability_invalid";
        return result;
    }
    const auto abilityId = document.value("ability_id", "");
    if (abilityId.empty()) {
        result.code = "project_reference_ability_invalid";
        return result;
    }
    addEdge(result.document, "ability", abilityId, "effect", document.value("effect_id", ""),
            "ability_effect", "effect", true);
    result.success = true;
    result.code = "project_reference_ability_extracted";
    return result;
}

ProjectReferenceExtractionResult extractCharacterReferences(const std::filesystem::path& document_path,
                                                            const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path;
    if (!document.is_object() || document.value("schemaVersion", "") != "1.0.0") {
        result.code = "project_reference_character_invalid";
        return result;
    }
    const auto characterId = document_path.stem().string();
    for (const auto& [field, target] : std::vector<std::pair<const char*, const char*>>{
             {"classId", "class"}, {"speciesId", "species"}, {"originId", "origin"},
             {"backgroundId", "background"}}) {
        addEdge(result.document, "character", characterId, target, document.value(field, ""),
                std::string("character_") + field, field);
    }
    for (const auto* field : {"portraitAssetId", "fieldSpriteAssetId", "battleSpriteAssetId"}) {
        addEdge(result.document, "character", characterId, "asset", document.value(field, ""),
                "character_appearance_asset", field, true);
    }
    const auto layers = document.value("layeredPartAssetIds", nlohmann::json::array());
    if (!layers.is_array()) {
        result.code = "project_reference_character_invalid";
        return result;
    }
    for (std::size_t index = 0; index < layers.size(); ++index) {
        const auto& value = layers[index];
        if (value.is_string()) addEdge(result.document, "character", characterId, "asset", value.get<std::string>(),
                                      "character_layer_asset", "layer:" + std::to_string(index), true);
    }
    result.success = true;
    result.code = "project_reference_character_extracted";
    return result;
}

ProjectReferenceExtractionResult extractVendorReferences(const std::filesystem::path& document_path,
                                                         const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path;
    if (!document.is_object() || !document.value("vendors", nlohmann::json::array()).is_array()) {
        result.code = "project_reference_vendor_invalid";
        return result;
    }
    for (const auto& vendor : document["vendors"]) {
        if (!vendor.is_object()) continue;
        const auto vendorId = vendor.value("id", "");
        const auto stockRows = vendor.value("stock", nlohmann::json::array());
        if (vendorId.empty() || !stockRows.is_array()) continue;
        for (std::size_t index = 0; index < stockRows.size(); ++index) {
            const auto& stock = stockRows[index];
            if (!stock.is_object()) continue;
            const auto local = "stock:" + std::to_string(index);
            addEdge(result.document, "vendor", vendorId, "item", stock.value("item_id", ""),
                    "vendor_stock_item", local, true);
            for (const auto& flag : stock.value("required_flags", nlohmann::json::array())) {
                if (flag.is_string()) addEdge(result.document, "vendor", vendorId, "flag", flag.get<std::string>(),
                                              "vendor_required_flag", local);
            }
        }
    }
    result.success = true;
    result.code = "project_reference_vendor_extracted";
    return result;
}

ProjectReferenceExtractionResult extractAudioMixReferences(const std::filesystem::path& document_path,
                                                           const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path;
    if (!document.is_object()) {
        result.code = "project_reference_audio_mix_invalid";
        return result;
    }
    addEdge(result.document, "audio_mix", "project", "asset", document.value("encounter_preview_asset_id", ""),
            "audio_preview_asset", "encounter_preview", true);
    result.success = true;
    result.code = "project_reference_audio_mix_extracted";
    return result;
}

ProjectReferenceExtractionResult extractDialogueReferences(const std::filesystem::path& document_path,
                                                            const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("schema_version", "") != "urpg.dialogue_graph.v1" ||
        !document.contains("nodes") || !document["nodes"].is_array()) {
        result.code = "project_reference_dialogue_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        const auto dialogueId = document_path.stem().string();
        for (const auto& node : document["nodes"]) {
            if (!node.is_object()) continue;
            const auto nodeId = node.value("id", "");
            if (nodeId.empty()) continue;
            const auto sourceId = dialogueId + "/" + nodeId;
            addEdge(result.document, "dialogue_node", sourceId, "dialogue", dialogueId, "owned_by_dialogue", "node:" + nodeId);
            addEdge(result.document, "dialogue_node", sourceId, "localization_key", node.value("localization_key", ""),
                    "node_text", "node:" + nodeId);
            addEdge(result.document, "dialogue_node", sourceId, "localization_key",
                    node.value("caption_localization_key", ""), "node_caption", "node:" + nodeId);
            addEdge(result.document, "dialogue_node", sourceId, "asset", node.value("voice_asset_id", ""),
                    "node_voice", "node:" + nodeId, true);
            for (const auto& choice : node.value("choices", nlohmann::json::array())) {
                if (!choice.is_object()) continue;
                const auto choiceId = choice.value("id", "");
                const auto local = "node:" + nodeId + ".choice:" + choiceId;
                const auto targetNodeId = choice.value("target_node_id", "");
                if (!targetNodeId.empty()) {
                    addEdge(result.document, "dialogue_node", sourceId, "dialogue_node",
                            dialogueId + "/" + targetNodeId, "choice_target", local);
                }
                addEdge(result.document, "dialogue_node", sourceId, "localization_key",
                        choice.value("localization_key", ""), "choice_text", local);
                for (const auto& condition : choice.value("conditions", nlohmann::json::array())) {
                    if (condition.is_object()) addEdge(result.document, "dialogue_node", sourceId, "variable",
                                                       condition.value("key", ""), "choice_condition", local);
                }
                for (const auto& effect : choice.value("effects", nlohmann::json::array())) {
                    if (effect.is_object()) addEdge(result.document, "dialogue_node", sourceId, "variable",
                                                    effect.value("key", ""), "choice_effect", local);
                }
            }
        }
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_dialogue_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_dialogue_document_extracted";
    return result;
}

ProjectReferenceExtractionResult extractQuestReferences(const std::filesystem::path& document_path,
                                                         const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("schema_version", "") != "urpg.quest_objective_graph.v1" ||
        document.value("quest_id", "").empty() || !document.contains("nodes") || !document["nodes"].is_array()) {
        result.code = "project_reference_quest_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        const auto questId = document["quest_id"].get<std::string>();
        for (const auto& node : document["nodes"]) {
            if (!node.is_object()) continue;
            const auto nodeId = node.value("id", "");
            if (nodeId.empty()) continue;
            const auto sourceId = questId + "/" + nodeId;
            const auto local = "node:" + nodeId;
            addEdge(result.document, "quest_node", sourceId, "quest", questId, "owned_by_quest", local);
            addEdge(result.document, "quest_node", sourceId, "localization_key", node.value("localization_key", ""),
                    "node_text", local);
            for (const auto& condition : node.value("conditions", nlohmann::json::array())) {
                if (condition.is_object()) addEdge(result.document, "quest_node", sourceId, condition.value("type", ""),
                                                   condition.value("id", ""), "objective_condition", local);
            }
            for (const auto& reward : node.value("rewards", nlohmann::json::array())) {
                if (reward.is_object()) addEdge(result.document, "quest_node", sourceId, reward.value("type", ""),
                                                reward.value("id", ""), "objective_reward", local);
            }
        }
        for (size_t index = 0; index < document.value("links", nlohmann::json::array()).size(); ++index) {
            const auto& link = document["links"][index];
            if (!link.is_object()) continue;
            const auto from = link.value("from", "");
            const auto to = link.value("to", "");
            if (!from.empty() && !to.empty()) addEdge(result.document, "quest_node", questId + "/" + from,
                                                       "quest_node", questId + "/" + to, "objective_link",
                                                       "link:" + std::to_string(index));
        }
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_quest_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_quest_document_extracted";
    return result;
}

ProjectReferenceExtractionResult extractMenuReferences(const std::filesystem::path& document_path,
                                                        const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("schema", "") != "urpg.menu_graph.v1" ||
        !document.contains("scenes") || !document["scenes"].is_array()) {
        result.code = "project_reference_menu_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        for (const auto& scene : document["scenes"]) {
            if (!scene.is_object()) continue;
            const auto sceneId = scene.value("scene_id", "");
            if (sceneId.empty()) continue;
            for (const auto& pane : scene.value("panes", nlohmann::json::array())) {
                if (!pane.is_object()) continue;
                const auto paneId = pane.value("id", "");
                for (const auto& command : pane.value("commands", nlohmann::json::array())) {
                    if (!command.is_object()) continue;
                    const auto commandId = command.value("id", "");
                    if (commandId.empty()) continue;
                    const auto sourceId = sceneId + "/" + paneId + "/" + commandId;
                    const auto local = "scene:" + sceneId + ".pane:" + paneId + ".command:" + commandId;
                    addEdge(result.document, "menu_command", sourceId, "menu_scene", sceneId, "owned_by_scene", local);
                    addEdge(result.document, "menu_command", sourceId, "asset", command.value("icon_id", ""),
                            "command_icon", local, true);
                    const auto customRoute = command.value("custom_route_id", "");
                    const auto route = customRoute.empty() ? command.value("route", "") : customRoute;
                    if (route != "None") addEdge(result.document, "menu_command", sourceId, "menu_route", route,
                                                  "command_route", local);
                    const auto fallbackCustom = command.value("fallback_custom_route_id", "");
                    const auto fallback = fallbackCustom.empty() ? command.value("fallback_route", "") : fallbackCustom;
                    if (fallback != "None") addEdge(result.document, "menu_command", sourceId, "menu_route", fallback,
                                                     "fallback_route", local);
                    for (const auto* ruleField : {"visibility_rules", "enable_rules"}) {
                        for (const auto& rule : command.value(ruleField, nlohmann::json::array())) {
                            if (!rule.is_object()) continue;
                            addEdge(result.document, "menu_command", sourceId, "switch", rule.value("switch_id", ""),
                                    std::string(ruleField) + "_switch", local);
                            addEdge(result.document, "menu_command", sourceId, "variable", rule.value("variable_id", ""),
                                    std::string(ruleField) + "_variable", local);
                        }
                    }
                }
            }
        }
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_menu_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_menu_document_extracted";
    return result;
}

ProjectReferenceExtractionResult extractDatabaseReferences(const std::filesystem::path& document_path,
                                                            const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("schema", "") != "urpg.database.v1" ||
        !document.contains("actors") || !document["actors"].is_array() ||
        !document.contains("items") || !document["items"].is_array()) {
        result.code = "project_reference_database_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        const auto databaseId = document_path.stem().string();
        for (const auto& actor : document["actors"]) {
            if (!actor.is_object()) continue;
            const auto actorId = actor.value("id", "");
            if (actorId.empty()) continue;
            const auto local = "actor:" + actorId;
            addEdge(result.document, "database", databaseId, "actor", actorId, "owns_actor", local);
            addEdge(result.document, "actor", actorId, "class", actor.value("class_id", ""), "actor_class", local);
        }
        for (const auto& item : document["items"]) {
            if (!item.is_object()) continue;
            const auto itemId = item.value("id", "");
            if (itemId.empty()) continue;
            addEdge(result.document, "database", databaseId, "item", itemId, "owns_item", "item:" + itemId);
        }
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_database_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_database_document_extracted";
    return result;
}

ProjectReferenceExtractionResult extractGameplayRecipeReferences(const std::filesystem::path& document_path,
                                                                  const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("schema_version", "") != "urpg.gameplay_recipe_project.v1" ||
        !document.contains("features") || !document["features"].is_array() ||
        !document.contains("recipe_receipts") || !document["recipe_receipts"].is_array()) {
        result.code = "project_reference_gameplay_recipe_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        const auto extractFeature = [&](const nlohmann::json& feature, const std::string& localPrefix,
                                        const bool packageInclusion) {
            if (!feature.is_object()) return;
            const auto featureId = feature.value("id", "");
            if (featureId.empty()) return;
            for (const auto& rule : feature.value("rules", nlohmann::json::array())) {
                if (!rule.is_object()) continue;
                const auto ruleId = rule.value("id", "");
                const auto sourceId = featureId + "/" + ruleId;
                const auto local = localPrefix + "feature:" + featureId + ".rule:" + ruleId;
                addEdge(result.document, "gameplay_rule", sourceId, "gameplay_target", rule.value("target", ""),
                        "rule_target", local, packageInclusion);
                for (const auto& flag : rule.value("required_flags", std::vector<std::string>{})) {
                    addEdge(result.document, "gameplay_rule", sourceId, "flag", flag, "requires_flag", local);
                }
                for (const auto& flag : rule.value("grants_flags", std::vector<std::string>{})) {
                    addEdge(result.document, "gameplay_rule", sourceId, "flag", flag, "grants_flag", local);
                }
                const auto variableWrites = rule.value("variable_writes", nlohmann::json::object());
                for (const auto& [variable, ignoredValue] : variableWrites.items()) {
                    addEdge(result.document, "gameplay_rule", sourceId, "variable", variable, "writes_variable", local);
                }
                const auto resourceDelta = rule.value("resource_delta", nlohmann::json::object());
                for (const auto& [resource, ignoredValue] : resourceDelta.items()) {
                    addEdge(result.document, "gameplay_rule", sourceId, "resource", resource, "changes_resource", local);
                }
            }
        };
        for (const auto& feature : document["features"]) extractFeature(feature, "", false);
        for (const auto& receipt : document["recipe_receipts"]) {
            if (!receipt.is_object()) continue;
            const auto recipeId = receipt.value("recipe_id", "");
            const auto targetId = receipt.value("target_id", "");
            addEdge(result.document, "recipe", recipeId, "gameplay_feature", targetId, "generated_feature",
                    "receipt:" + recipeId, true);
            extractFeature(receipt.value("applied_target", nlohmann::json::object()), "receipt:" + recipeId + ".", true);
        }
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_gameplay_recipe_document_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_gameplay_recipe_document_extracted";
    return result;
}

ProjectReferenceExtractionResult extractMzPluginLockReferences(const std::filesystem::path& document_path,
                                                               const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("schema_version", "") != "urpg.mz_plugin_static_lock.v1" ||
        document.value("inspection_only", false) != true || !document.contains("plugins") ||
        !document["plugins"].is_array() || !document.contains("load_order") || !document["load_order"].is_array()) {
        result.code = "project_reference_mz_plugin_lock_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        const auto packageId = document_path.stem().string();
        std::string previousPlugin;
        for (const auto& pluginIdJson : document["load_order"]) {
            const auto pluginId = pluginIdJson.get<std::string>();
            if (!previousPlugin.empty()) {
                addEdge(result.document, "plugin", pluginId, "plugin", previousPlugin, "loads_after",
                        "load_order:" + pluginId);
            }
            previousPlugin = pluginId;
        }
        for (const auto& plugin : document["plugins"]) {
            if (!plugin.is_object()) continue;
            const auto pluginId = plugin.value("plugin_id", "");
            if (pluginId.empty()) continue;
            const auto local = "plugin:" + pluginId;
            if (plugin.value("enabled", false)) {
                addEdge(result.document, "plugin_package", packageId, "plugin", pluginId, "enabled_plugin", local, true);
            }
            for (const auto& dependency : plugin.value("dependencies", nlohmann::json::array())) {
                if (!dependency.is_object()) continue;
                addEdge(result.document, "plugin", pluginId, "plugin", dependency.value("plugin_id", ""),
                        dependency.value("optional", false) ? "optional_dependency" : "required_dependency", local,
                        plugin.value("enabled", false) && !dependency.value("optional", false));
            }
        }
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_mz_plugin_lock_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_mz_plugin_lock_extracted";
    return result;
}

ProjectReferenceExtractionResult extractModManifestReferences(const std::filesystem::path& document_path,
                                                              const nlohmann::json& document) {
    ProjectReferenceExtractionResult result;
    result.document.document_path = document_path.lexically_normal();
    if (!document.is_object() || document.value("id", "").empty() || document.value("name", "").empty() ||
        document.value("version", "").empty() || !document.contains("dependencies") ||
        !document["dependencies"].is_array()) {
        result.code = "project_reference_mod_manifest_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    try {
        const auto modId = document["id"].get<std::string>();
        const auto local = "mod:" + modId;
        for (const auto& dependency : document["dependencies"]) {
            addEdge(result.document, "mod", modId, "mod", dependency.get<std::string>(), "required_dependency",
                    local, true);
        }
        addEdge(result.document, "mod", modId, "script", document.value("entryPoint", ""), "entry_point", local,
                true);
    } catch (const nlohmann::json::exception&) {
        result.document.edges.clear();
        result.code = "project_reference_mod_manifest_invalid";
        result.diagnostics.push_back(result.code);
        return result;
    }
    result.success = true;
    result.code = "project_reference_mod_manifest_extracted";
    return result;
}

ProjectReferenceExtractionResult extractProjectDocumentReferences(const std::filesystem::path& project_root,
                                                                   const std::filesystem::path& document_path,
                                                                   const nlohmann::json& document) {
    const auto normalized = document_path.lexically_normal();
    const auto relative = normalized.lexically_relative(project_root.lexically_normal()).generic_string();
    if (relative.starts_with("content/maps/") && relative.ends_with(".p2d.json")) {
        return extractPerspective2DReferences(normalized, document);
    }
    if (relative.starts_with("content/maps/") && relative.ends_with(".grid.json")) {
        return extractGridPartReferences(normalized, document);
    }
    if (relative.starts_with("content/abilities/") && relative.ends_with(".json")) {
        return extractAbilityReferences(normalized, document);
    }
    if (relative.starts_with("content/characters/") && relative.ends_with(".json")) {
        return extractCharacterReferences(normalized, document);
    }
    if (relative.starts_with("content/vendors/") && relative.ends_with(".json")) {
        return extractVendorReferences(normalized, document);
    }
    if (relative == "config/audio_mix_presets.json") return extractAudioMixReferences(normalized, document);
    if (relative.starts_with("content/dialogues/") && relative.ends_with(".json")) {
        return extractDialogueReferences(normalized, document);
    }
    if (relative.starts_with("content/quests/") && relative.ends_with(".json")) {
        return extractQuestReferences(normalized, document);
    }
    if (relative == "content/ui/menus.json") return extractMenuReferences(normalized, document);
    if (relative == "content/database.json") return extractDatabaseReferences(normalized, document);
    if (relative == "content/gameplay/recipes.json") return extractGameplayRecipeReferences(normalized, document);
    if (relative == "content/compat/mz_plugin_lock.json") return extractMzPluginLockReferences(normalized, document);
    if (relative.starts_with("mods/") &&
        (normalized.filename() == "mod.json" || normalized.filename() == "manifest.json")) {
        return extractModManifestReferences(normalized, document);
    }
    return {false, "project_reference_document_kind_unsupported", {normalized, {}},
            {"No stable-reference extractor owns this document path."}};
}

ProjectReferenceBuildResult buildProjectReferenceIndex(const std::filesystem::path& project_root) {
    ProjectReferenceBuildResult result;
    if (project_root.empty()) {
        result.diagnostics.push_back("project_reference_project_root_missing");
        return result;
    }

    enum class DocumentKind { GridPart, Perspective2D, Ability, Character, Dialogue, Quest, Vendor, AudioMix,
                              Menu, Database, Gameplay, MzPluginLock, ModManifest };
    std::vector<std::pair<std::filesystem::path, DocumentKind>> candidates;
    const auto addFile = [&](const std::filesystem::path& path, const DocumentKind kind) {
        std::error_code error;
        if (std::filesystem::is_regular_file(path, error) && !error) candidates.emplace_back(path, kind);
    };
    const auto addJsonDirectory = [&](const std::filesystem::path& directory, const DocumentKind kind,
                                      const bool perspectiveOnly = false) {
        std::error_code error;
        if (!std::filesystem::exists(directory, error)) return;
        std::filesystem::directory_iterator iterator(directory, error);
        const std::filesystem::directory_iterator end;
        for (; !error && iterator != end; iterator.increment(error)) {
            const auto& entry = *iterator;
            const auto filename = entry.path().filename().string();
            if (entry.is_regular_file() && entry.path().extension() == ".json" &&
                (!perspectiveOnly || filename.ends_with(".p2d.json"))) {
                candidates.emplace_back(entry.path(), kind);
            }
        }
        if (error) result.diagnostics.push_back("project_reference_directory_unreadable:" + directory.generic_string());
    };

    addJsonDirectory(project_root / "content" / "maps", DocumentKind::Perspective2D, true);
    addJsonDirectory(project_root / "content" / "maps", DocumentKind::GridPart);
    std::erase_if(candidates, [](const auto& candidate) {
        return candidate.second == DocumentKind::GridPart &&
               !candidate.first.filename().string().ends_with(".grid.json");
    });
    addJsonDirectory(project_root / "content" / "abilities", DocumentKind::Ability);
    addJsonDirectory(project_root / "content" / "characters", DocumentKind::Character);
    addJsonDirectory(project_root / "content" / "dialogues", DocumentKind::Dialogue);
    addJsonDirectory(project_root / "content" / "quests", DocumentKind::Quest);
    addJsonDirectory(project_root / "content" / "vendors", DocumentKind::Vendor);
    addFile(project_root / "config" / "audio_mix_presets.json", DocumentKind::AudioMix);
    addFile(project_root / "content" / "ui" / "menus.json", DocumentKind::Menu);
    addFile(project_root / "content" / "database.json", DocumentKind::Database);
    addFile(project_root / "content" / "gameplay" / "recipes.json", DocumentKind::Gameplay);
    addFile(project_root / "content" / "compat" / "mz_plugin_lock.json", DocumentKind::MzPluginLock);

    std::error_code modError;
    const auto mods = project_root / "mods";
    if (std::filesystem::exists(mods, modError)) {
        std::filesystem::recursive_directory_iterator iterator(mods, modError);
        const std::filesystem::recursive_directory_iterator end;
        for (; !modError && iterator != end; iterator.increment(modError)) {
            const auto& entry = *iterator;
            const auto filename = entry.path().filename().string();
            if (entry.is_regular_file() && (filename == "mod.json" || filename == "manifest.json")) {
                candidates.emplace_back(entry.path(), DocumentKind::ModManifest);
            }
        }
        if (modError) result.diagnostics.push_back("project_reference_directory_unreadable:" + mods.generic_string());
    }

    std::sort(candidates.begin(), candidates.end(), [](const auto& left, const auto& right) {
        return std::tie(left.first, left.second) < std::tie(right.first, right.second);
    });
    for (const auto& [path, kind] : candidates) {
        std::ifstream input(path, std::ios::binary);
        const auto document = nlohmann::json::parse(input, nullptr, false);
        if (!input.good() && !input.eof()) {
            result.diagnostics.push_back("project_reference_document_unreadable:" + path.generic_string());
            continue;
        }
        if (document.is_discarded()) {
            result.diagnostics.push_back("project_reference_document_json_invalid:" + path.generic_string());
            continue;
        }
        ProjectReferenceExtractionResult extracted;
        switch (kind) {
        case DocumentKind::GridPart: extracted = extractGridPartReferences(path, document); break;
        case DocumentKind::Perspective2D: extracted = extractPerspective2DReferences(path, document); break;
        case DocumentKind::Ability: extracted = extractAbilityReferences(path, document); break;
        case DocumentKind::Character: extracted = extractCharacterReferences(path, document); break;
        case DocumentKind::Dialogue: extracted = extractDialogueReferences(path, document); break;
        case DocumentKind::Quest: extracted = extractQuestReferences(path, document); break;
        case DocumentKind::Vendor: extracted = extractVendorReferences(path, document); break;
        case DocumentKind::AudioMix: extracted = extractAudioMixReferences(path, document); break;
        case DocumentKind::Menu: extracted = extractMenuReferences(path, document); break;
        case DocumentKind::Database: extracted = extractDatabaseReferences(path, document); break;
        case DocumentKind::Gameplay: extracted = extractGameplayRecipeReferences(path, document); break;
        case DocumentKind::MzPluginLock: extracted = extractMzPluginLockReferences(path, document); break;
        case DocumentKind::ModManifest: extracted = extractModManifestReferences(path, document); break;
        }
        if (!extracted.success) {
            result.diagnostics.push_back(extracted.code + ":" + path.generic_string());
            continue;
        }
        result.documents.push_back(std::move(extracted.document));
    }
    const auto rebuilt = result.index.rebuild(result.documents);
    if (!rebuilt.success) result.diagnostics.push_back(rebuilt.code);
    result.success = result.diagnostics.empty() && rebuilt.success;
    return result;
}

} // namespace urpg::project
