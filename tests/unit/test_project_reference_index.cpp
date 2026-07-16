#include "engine/core/project/project_reference_index.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

using urpg::project::ProjectReferenceDocument;
using urpg::project::ProjectReferenceEdge;
using urpg::project::ProjectReferenceIndex;

namespace {

ProjectReferenceEdge edge(std::string sourceType, std::string sourceId, std::string targetType,
                          std::string targetId, std::string referenceType, std::string localId,
                          const bool packageInclusion = false) {
    return {std::move(sourceType), std::move(sourceId), std::move(targetType), std::move(targetId),
            std::move(referenceType), {}, std::move(localId), packageInclusion};
}

} // namespace

TEST_CASE("ProjectReferenceIndex rebuild and incremental replacement are identical", "[project][reference_index]") {
    const ProjectReferenceDocument map{
        "content/maps/start.p2d.json",
        {edge("map", "start", "asset", "asset.grass", "tile_asset", "tile:0", true),
         edge("event", "welcome", "switch", "intro_seen", "condition_switch", "page:main")}};
    const ProjectReferenceDocument dialogue{
        "content/dialogues/intro.json",
        {edge("dialogue", "intro", "asset", "voice.guide", "voice_asset", "node:start", true),
         edge("dialogue", "intro", "variable", "rank", "choice_condition", "choice:ranked")}};

    ProjectReferenceIndex rebuilt;
    REQUIRE(rebuilt.rebuild({dialogue, map}).success);

    ProjectReferenceIndex incremental;
    REQUIRE(incremental.replaceDocument(map).success);
    REQUIRE(incremental.replaceDocument(dialogue).success);
    REQUIRE(incremental.edges() == rebuilt.edges());
    REQUIRE(incremental.inbound("asset", "voice.guide").size() == 1);
    REQUIRE(incremental.outbound("event", "welcome").size() == 1);
    REQUIRE(incremental.whyIncluded("asset", "asset.grass").size() == 1);
    REQUIRE(incremental.whyIncluded("switch", "intro_seen").empty());
    const auto uses = incremental.findUses("asset", "voice.guide");
    REQUIRE(uses.success);
    REQUIRE(uses.matches[0].document_path == "content/dialogues/intro.json");
    REQUIRE(uses.matches[0].local_id == "node:start");
    REQUIRE(uses.matches[0].reference_type == "voice_asset");
    const auto references = incremental.findReferences("event", "welcome");
    REQUIRE(references.success);
    REQUIRE(references.matches[0].target_id == "intro_seen");
    const auto inclusion = incremental.explainInclusion("asset", "asset.grass");
    REQUIRE(inclusion.success);
    REQUIRE(inclusion.matches.size() == 1);
    REQUIRE_FALSE(incremental.findUses("", "voice.guide").success);
}

TEST_CASE("ProjectReferenceIndex rejects invalid replacement without partial mutation", "[project][reference_index]") {
    ProjectReferenceIndex index;
    REQUIRE(index.replaceDocument({"content/maps/start.p2d.json",
                                   {edge("map", "start", "asset", "asset.grass", "tile_asset", "tile:0")}})
                .success);
    const auto before = index.edges();
    const auto rejected = index.replaceDocument({"content/maps/start.p2d.json",
                                                  {edge("map", "start", "asset", "", "tile_asset", "tile:0")}});
    REQUIRE_FALSE(rejected.success);
    REQUIRE(rejected.code == "project_reference_edge_invalid");
    REQUIRE(index.edges() == before);
}

TEST_CASE("ProjectReferenceIndex extracts map event and dialogue stable-ID edges", "[project][reference_index]") {
    const nlohmann::json map = {
        {"document_kind", "urpg.perspective_2d.map"}, {"version", 1}, {"map_id", "start"},
        {"tile_palette", {{{"option_id", "grass"}, {"asset_id", "asset.grass"}}}},
        {"events", {{{"event_id", "welcome"}, {"asset_id", "asset.guide"},
                      {"pages", {{{"page_id", "main"},
                                  {"conditions", {{{"type", "switch"}, {"key", "intro_ready"}}}},
                                  {"commands", {{{"code", "start_dialogue"}, {"argument", "intro"}},
                                                {{"code", "change_variable"}, {"argument", "rank+=1"}}}}}}}}}}};
    const nlohmann::json dialogue = {
        {"schema_version", "urpg.dialogue_graph.v1"},
        {"nodes", {{{"id", "start"}, {"localization_key", "dialogue.intro"},
                     {"voice_asset_id", "voice.guide"},
                     {"choices", {{{"id", "continue"}, {"target_node_id", "end"},
                                    {"conditions", {{{"key", "rank"}}}}, {"effects", nlohmann::json::array()}}}}},
                    {{"id", "end"}, {"localization_key", "dialogue.end"},
                     {"choices", nlohmann::json::array()}}}}};
    const auto mapExtracted = urpg::project::extractPerspective2DReferences("content/maps/start.p2d.json", map);
    const auto dialogueExtracted = urpg::project::extractDialogueReferences("content/dialogues/intro.json", dialogue);
    REQUIRE(mapExtracted.success);
    REQUIRE(dialogueExtracted.success);

    ProjectReferenceIndex rebuilt;
    REQUIRE(rebuilt.rebuild({mapExtracted.document, dialogueExtracted.document}).success);
    ProjectReferenceIndex incremental;
    REQUIRE(incremental.replaceDocument(mapExtracted.document).success);
    REQUIRE(incremental.replaceDocument(dialogueExtracted.document).success);
    REQUIRE(incremental.edges() == rebuilt.edges());
    REQUIRE(incremental.inbound("switch", "intro_ready").size() == 1);
    REQUIRE(incremental.inbound("variable", "rank").size() == 2);
    const auto introUses = incremental.inbound("dialogue", "intro");
    REQUIRE(introUses.size() == 3);
    REQUIRE(std::count_if(introUses.begin(), introUses.end(), [](const auto& use) {
                return use.reference_type == "start_dialogue";
            }) == 1);
    REQUIRE(incremental.inbound("asset", "voice.guide").size() == 1);
    REQUIRE(incremental.whyIncluded("asset", "asset.grass").size() == 1);
}

TEST_CASE("ProjectReferenceIndex extracts quest and menu stable-ID edges", "[project][reference_index]") {
    const nlohmann::json quest = {
        {"schema_version", "urpg.quest_objective_graph.v1"}, {"quest_id", "relic"},
        {"nodes", {{{"id", "find"}, {"localization_key", "quest.relic.find"},
                     {"conditions", {{{"type", "item"}, {"id", "ancient_relic"}},
                                     {{"type", "switch"}, {"id", "relic_found"}}}},
                     {"rewards", {{{"type", "currency"}, {"id", "gold"}}}}},
                    {{"id", "end"}, {"conditions", nlohmann::json::array()},
                     {"rewards", nlohmann::json::array()}}}},
        {"links", {{{"from", "find"}, {"to", "end"}}}}};
    const auto menu = nlohmann::json::parse(R"json({
        "schema": "urpg.menu_graph.v1",
        "scenes": [{
            "scene_id": "main",
            "panes": [{
                "id": "commands",
                "commands": [{
                    "id": "journal",
                    "icon_id": "asset.icon.journal",
                    "route": "Custom",
                    "custom_route_id": "journal",
                    "fallback_route": "None",
                    "visibility_rules": [{"switch_id": "journal_open"}],
                    "enable_rules": [{"variable_id": "chapter"}]
                }]
            }]
        }]
    })json");
    const auto questExtracted = urpg::project::extractQuestReferences("content/quests/relic.json", quest);
    const auto menuExtracted = urpg::project::extractMenuReferences("content/ui/menus.json", menu);
    REQUIRE(questExtracted.success);
    REQUIRE(menuExtracted.success);
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({menuExtracted.document, questExtracted.document}).success);
    REQUIRE(index.inbound("item", "ancient_relic").size() == 1);
    REQUIRE(index.inbound("switch", "relic_found").size() == 1);
    REQUIRE(index.inbound("quest_node", "relic/end").size() == 1);
    REQUIRE(index.inbound("menu_route", "journal").size() == 1);
    REQUIRE(index.inbound("switch", "journal_open").size() == 1);
    REQUIRE(index.inbound("variable", "chapter").size() == 1);
    REQUIRE(index.whyIncluded("asset", "asset.icon.journal").size() == 1);
}

TEST_CASE("ProjectReferenceIndex extracts database gameplay and plugin edges", "[project][reference_index]") {
    const nlohmann::json database = {
        {"schema", "urpg.database.v1"},
        {"actors", {{{"id", "hero"}, {"class_id", "guardian"}}}},
        {"items", {{{"id", "potion"}}}}};
    const nlohmann::json rule = {
        {"id", "gain_momentum"}, {"target", "party"},
        {"required_flags", {"battle_started"}}, {"grants_flags", {"momentum_ready"}},
        {"variable_writes", {{"combo", "1"}}}, {"resource_delta", {{"mp", -2}}}};
    const nlohmann::json feature = {
        {"schema_version", "urpg.gameplay_wysiwyg.v1"}, {"id", "momentum"},
        {"feature_type", "status_effect_designer"}, {"rules", {rule}}};
    const nlohmann::json gameplay = {
        {"schema_version", "urpg.gameplay_recipe_project.v1"}, {"features", {feature}},
        {"recipe_receipts", {{{"recipe_id", "starter.momentum"}, {"target_id", "momentum"},
                               {"applied_target", feature}}}}};
    const nlohmann::json plugins = {
        {"schema_version", "urpg.mz_plugin_static_lock.v1"}, {"inspection_only", true},
        {"load_order", {"Core", "BattlePlus"}},
        {"plugins", {{{"plugin_id", "Core"}, {"enabled", true},
                       {"dependencies", nlohmann::json::array()}},
                      {{"plugin_id", "BattlePlus"}, {"enabled", true},
                       {"dependencies", {{{"plugin_id", "Core"}, {"optional", false}}}}}}}};

    const auto databaseExtracted = urpg::project::extractDatabaseReferences("data/database.json", database);
    const auto gameplayExtracted = urpg::project::extractGameplayRecipeReferences("content/gameplay.json", gameplay);
    const auto pluginsExtracted = urpg::project::extractMzPluginLockReferences("js/plugins.lock.json", plugins);
    REQUIRE(databaseExtracted.success);
    REQUIRE(gameplayExtracted.success);
    REQUIRE(pluginsExtracted.success);
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({databaseExtracted.document, gameplayExtracted.document, pluginsExtracted.document}).success);
    REQUIRE(index.inbound("class", "guardian").size() == 1);
    REQUIRE(index.inbound("flag", "battle_started").size() == 2);
    REQUIRE(index.inbound("flag", "momentum_ready").size() == 2);
    REQUIRE(index.inbound("variable", "combo").size() == 2);
    REQUIRE(index.inbound("resource", "mp").size() == 2);
    REQUIRE(index.whyIncluded("gameplay_feature", "momentum").size() == 1);
    REQUIRE(index.inbound("plugin", "Core").size() == 3);
    REQUIRE(index.whyIncluded("plugin", "Core").size() == 2);
    REQUIRE(index.whyIncluded("plugin", "BattlePlus").size() == 1);
}

TEST_CASE("ProjectReferenceIndex extracts native mod package edges", "[project][reference_index]") {
    const nlohmann::json manifest = {
        {"id", "battle_overhaul"}, {"name", "Battle Overhaul"}, {"version", "1.2.0"},
        {"dependencies", {"core_rules"}}, {"entryPoint", "mods/battle_overhaul/main.js"}};
    const auto extracted = urpg::project::extractModManifestReferences("mods/battle_overhaul/mod.json", manifest);
    REQUIRE(extracted.success);
    ProjectReferenceIndex index;
    REQUIRE(index.replaceDocument(extracted.document).success);
    REQUIRE(index.inbound("mod", "core_rules").size() == 1);
    REQUIRE(index.whyIncluded("mod", "core_rules").size() == 1);
    REQUIRE(index.whyIncluded("script", "mods/battle_overhaul/main.js").size() == 1);
}

TEST_CASE("ProjectReferenceIndex project rebuild matches document incremental replay", "[project][reference_index]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_project_reference_index_rebuild_test";
    std::error_code cleanupError;
    std::filesystem::remove_all(root, cleanupError);
    std::filesystem::create_directories(root / "content");
    std::filesystem::create_directories(root / "mods" / "battle_overhaul");
    const nlohmann::json database = {
        {"schema", "urpg.database.v1"},
        {"actors", {{{"id", "hero"}, {"class_id", "guardian"}}}},
        {"items", {{{"id", "potion"}}}}};
    const nlohmann::json manifest = {
        {"id", "battle_overhaul"}, {"name", "Battle Overhaul"}, {"version", "1.2.0"},
        {"dependencies", {"core_rules"}}, {"entryPoint", "mods/battle_overhaul/main.js"}};
    std::ofstream(root / "content" / "database.json", std::ios::binary) << database.dump(2);
    std::ofstream(root / "mods" / "battle_overhaul" / "mod.json", std::ios::binary) << manifest.dump(2);

    const auto rebuilt = urpg::project::buildProjectReferenceIndex(root);
    REQUIRE(rebuilt.success);
    REQUIRE(rebuilt.documents.size() == 2);
    ProjectReferenceIndex incremental;
    for (const auto& document : rebuilt.documents) REQUIRE(incremental.replaceDocument(document).success);
    REQUIRE(incremental.edges() == rebuilt.index.edges());
    REQUIRE(rebuilt.index.inbound("class", "guardian").size() == 1);
    REQUIRE(rebuilt.index.whyIncluded("mod", "core_rules").size() == 1);

    std::filesystem::remove_all(root, cleanupError);
}
