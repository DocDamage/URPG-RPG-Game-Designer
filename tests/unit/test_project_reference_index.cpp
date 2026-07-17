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

TEST_CASE("ProjectReferenceIndex narrows stable object queries through maintained postings",
          "[project][reference_index][pcq701][perf][primary_routes]") {
    ProjectReferenceDocument document;
    document.document_path = "content/maps/large.p2d.json";
    for (size_t index = 0; index < 256; ++index) {
        document.edges.push_back({"event", "map_001/event_" + std::to_string(index), "asset",
                                  "asset_" + std::to_string(index), "event_sprite_asset", {},
                                  "event:" + std::to_string(index), index % 2 == 0});
    }

    ProjectReferenceIndex index;
    REQUIRE(index.replaceDocument(document).success);
    REQUIRE(index.indexRevision() == 1);

    const auto uses = index.findUses("asset", "asset_193");
    REQUIRE(uses.success);
    REQUIRE(uses.matches.size() == 1);
    REQUIRE(uses.matches.front().source_id == "map_001/event_193");
    REQUIRE(index.lastQueryCandidateCount() == 1);

    const auto references = index.findReferences("event", "map_001/event_64");
    REQUIRE(references.success);
    REQUIRE(references.matches.size() == 1);
    REQUIRE(references.matches.front().target_id == "asset_64");
    REQUIRE(index.lastQueryCandidateCount() == 1);

    const auto included = index.explainInclusion("asset", "asset_64");
    REQUIRE(included.success);
    REQUIRE(included.matches.size() == 1);
    REQUIRE(index.lastQueryCandidateCount() == 1);

    const auto excluded = index.explainInclusion("asset", "asset_193");
    REQUIRE(excluded.success);
    REQUIRE(excluded.matches.empty());
    REQUIRE(index.lastQueryCandidateCount() == 0);
    REQUIRE_FALSE(index.findUses("", "asset_193").success);
    REQUIRE(index.lastQueryCandidateCount() == 0);

    const auto revision = index.indexRevision();
    index.removeDocument(document.document_path);
    REQUIRE(index.indexRevision() == revision + 1);
    REQUIRE(index.findUses("asset", "asset_193").matches.empty());
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

TEST_CASE("ProjectReferenceIndex extracts grid ability character vendor and audio owner edges",
          "[project][reference_index][owners]") {
    const auto grid = urpg::project::extractGridPartReferences(
        "content/maps/start.grid.json",
        {{"schemaVersion", 1}, {"mapId", "start"},
         {"parts", {{{"instanceId", "hero"}, {"partId", "part.player"},
                       {"properties", {{"portrait_asset_id", "asset.hero.portrait"}}}}}}});
    const auto ability = urpg::project::extractAbilityReferences(
        "content/abilities/fire.json", {{"ability_id", "fire"}, {"effect_id", "burn"}});
    const auto character = urpg::project::extractCharacterReferences(
        "content/characters/hero.json",
        {{"schemaVersion", "1.0.0"}, {"classId", "guardian"}, {"speciesId", "human"},
         {"originId", "willow"}, {"backgroundId", "warden"},
         {"portraitAssetId", "asset.hero.portrait"}, {"fieldSpriteAssetId", "asset.hero.field"},
         {"battleSpriteAssetId", "asset.hero.battle"},
         {"layeredPartAssetIds", {"asset.hero.hat"}}});
    const auto vendor = urpg::project::extractVendorReferences(
        "content/vendors/village.json",
        nlohmann::json::parse(R"({"vendors":[{"id":"village","stock":[{"item_id":"potion","required_flags":["shop_open"]}]}]})"));
    const auto audio = urpg::project::extractAudioMixReferences(
        "config/audio_mix_presets.json", {{"version", "1.0.0"},
                                           {"encounter_preview_asset_id", "asset.battle.theme"}});
    REQUIRE(grid.success);
    REQUIRE(ability.success);
    REQUIRE(character.success);
    REQUIRE(vendor.success);
    REQUIRE(audio.success);

    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({grid.document, ability.document, character.document, vendor.document, audio.document}).success);
    REQUIRE(index.inbound("grid_part", "part.player").size() == 1);
    REQUIRE(index.inbound("effect", "burn").size() == 1);
    REQUIRE(index.inbound("class", "guardian").size() == 1);
    REQUIRE(index.inbound("asset", "asset.hero.portrait").size() == 2);
    REQUIRE(index.inbound("item", "potion").size() == 1);
    REQUIRE(index.inbound("flag", "shop_open").size() == 1);
    REQUIRE(index.whyIncluded("asset", "asset.battle.theme").size() == 1);
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

TEST_CASE("Project reference document dispatcher incrementally replaces a live owning document",
          "[project][reference_index][incremental]") {
    const std::filesystem::path root = "C:/projects/demo";
    const auto path = root / "content" / "dialogues" / "intro.json";
    const auto initial = nlohmann::json{
        {"schema_version", "urpg.dialogue_graph.v1"},
        {"nodes", {{{"id", "start"}, {"localization_key", "dialogue.start"},
                     {"voice_asset_id", "voice.old"}, {"choices", nlohmann::json::array()}}}}};
    const auto changed = nlohmann::json{
        {"schema_version", "urpg.dialogue_graph.v1"},
        {"nodes", {{{"id", "start"}, {"localization_key", "dialogue.start"},
                     {"voice_asset_id", "voice.new"}, {"choices", nlohmann::json::array()}}}}};

    ProjectReferenceIndex incremental;
    const auto first = urpg::project::extractProjectDocumentReferences(root, path, initial);
    REQUIRE(first.success);
    REQUIRE(incremental.replaceDocument(first.document).success);
    REQUIRE(incremental.inbound("asset", "voice.old").size() == 1);
    const auto replacement = urpg::project::extractProjectDocumentReferences(root, path, changed);
    REQUIRE(replacement.success);
    REQUIRE(incremental.replaceDocument(replacement.document).success);
    REQUIRE(incremental.inbound("asset", "voice.old").empty());
    REQUIRE(incremental.inbound("asset", "voice.new").size() == 1);

    ProjectReferenceIndex rebuilt;
    REQUIRE(rebuilt.rebuild({replacement.document}).success);
    REQUIRE(incremental.edges() == rebuilt.edges());
    REQUIRE_FALSE(urpg::project::extractProjectDocumentReferences(
                      root, root / "content" / "unknown.json", nlohmann::json::object()).success);
}

TEST_CASE("ProjectReferenceIndex global object search is indexed deterministic and navigable",
          "[project][reference_index][global_search]") {
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({
        {"content/maps/start.p2d.json",
         {{"map", "start", "asset", "asset.hero.portrait", "portrait", {}, "event:hero", true},
          {"event", "guide", "dialogue", "moonwell_intro", "start_dialogue", {}, "command:0", true}}},
        {"content/dialogues/moonwell_intro.json",
         {{"dialogue", "moonwell_intro", "asset", "asset.guide.voice", "voice", {}, "node:intro", true}}},
    }).success);

    const auto dialogue = index.searchObjects("moonwell");
    REQUIRE(dialogue.size() == 1);
    REQUIRE(dialogue[0].object_type == "dialogue");
    REQUIRE(dialogue[0].object_id == "moonwell_intro");
    REQUIRE(dialogue[0].document_path == "content/dialogues/moonwell_intro.json");
    REQUIRE(dialogue[0].inbound_count == 1);
    REQUIRE(dialogue[0].outbound_count == 1);
    REQUIRE(dialogue[0].package_included);
    REQUIRE(index.lastQueryCandidateCount() == 1);

    const auto assets = index.searchObjects("asset", 2);
    REQUIRE(assets.size() == 2);
    REQUIRE(assets[0].object_type == "asset");
    REQUIRE(assets[0].object_id == "asset.guide.voice");
    REQUIRE(assets[1].object_id == "asset.hero.portrait");
    REQUIRE(index.searchObjects("does-not-exist").empty());
    REQUIRE(index.searchObjects({}, 0).empty());
}

TEST_CASE("ProjectReferenceIndex resolves canonical owning workspace navigation",
          "[project][reference_index][navigation]") {
    ProjectReferenceIndex index;
    std::vector<ProjectReferenceEdge> edges;
    const std::vector<std::pair<std::string, std::string>> expectedPanels = {
        {"asset", "assets"},
        {"plugin", "mod"},
        {"plugin_package", "mod"},
        {"mod", "mod"},
        {"script", "mod"},
        {"ability", "ability"},
        {"effect", "ability"},
        {"gameplay_feature", "ability"},
        {"recipe", "ability"},
        {"character", "character_creator"},
        {"map", "spatial_authoring"},
        {"grid_part_instance", "spatial_authoring"},
        {"grid_part", "spatial_authoring"},
        {"event", "spatial_authoring"},
        {"dialogue", "spatial_authoring"},
        {"dialogue_node", "spatial_authoring"},
        {"quest", "spatial_authoring"},
        {"quest_node", "spatial_authoring"},
        {"database_item", "diagnostics"},
    };
    for (size_t i = 0; i < expectedPanels.size(); ++i) {
        const auto& [type, panel] = expectedPanels[i];
        (void)panel;
        edges.push_back({type, type + ".object", "asset", "asset.target." + std::to_string(i),
                         "test_reference", {}, "local:" + std::to_string(i), false});
    }
    REQUIRE(index.rebuild({{"content/navigation_contract.json", std::move(edges)}}).success);

    for (const auto& [type, panel] : expectedPanels) {
        const auto target = index.navigationTarget(type, type + ".object");
        CAPTURE(type, panel, target.code);
        REQUIRE(target.success);
        REQUIRE(target.panel_id == panel);
        REQUIRE(target.document_path == "content/navigation_contract.json");
        REQUIRE_FALSE(target.local_id.empty());
        REQUIRE(target.object_type == type);
        REQUIRE(target.object_id == type + ".object");
    }

    REQUIRE_FALSE(index.navigationTarget({}, "object").success);
    REQUIRE(index.navigationTarget({}, "object").code == "project_reference_navigation_object_missing");
    REQUIRE_FALSE(index.navigationTarget("map", "missing").success);
    REQUIRE(index.navigationTarget("map", "missing").code ==
            "project_reference_navigation_object_not_indexed");
}
