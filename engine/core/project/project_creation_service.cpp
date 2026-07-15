#include "engine/core/project/project_creation_service.h"

#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/project/project_template_generator.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace urpg::project {

namespace {

bool writeJson(const std::filesystem::path& path, const nlohmann::json& value) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return false;
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << value.dump(2) << '\n';
    return static_cast<bool>(output);
}

ProjectCreationResult failure(std::string code, std::string message) {
    ProjectCreationResult result;
    result.code = std::move(code);
    result.message = std::move(message);
    result.errors.push_back(result.code);
    return result;
}

bool isSafeIdentifier(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](const unsigned char character) {
        return std::isalnum(character) || character == '_' || character == '-';
    });
}

} // namespace

ProjectCreationResult ProjectCreationService::createProject(const ProjectCreationRequest& request) const {
    if (request.destination.empty() || request.destination.filename().empty()) {
        return failure("project_destination_invalid", "Choose an empty destination directory for the new project.");
    }
    if (std::filesystem::exists(request.destination)) {
        return failure("project_destination_exists", "The selected project destination already exists.");
    }
    if (!isSafeIdentifier(request.starter_map)) {
        return failure("project_starter_map_invalid",
                       "Choose a starter-map ID containing only letters, numbers, underscores, or hyphens.");
    }
    if (request.include_creator_vertical_slice_seed && request.starter_map != "willow_village") {
        return failure("project_vertical_slice_seed_starter_map_invalid",
                       "Lantern of the Willow must start at the willow_village map.");
    }

    ProjectTemplateGenerator generator;
    const auto generated = generator.generate({request.template_id, request.project_id, request.project_name});
    if (!generated.success) {
        auto result = failure("project_template_invalid", "The selected project template could not be generated.");
        result.errors = generated.errors;
        return result;
    }

    const auto nonce = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto temporary_root = request.destination.parent_path() /
                                ("." + request.destination.filename().string() + ".creating-" + nonce);
    std::error_code error;
    std::filesystem::create_directories(temporary_root, error);
    if (error) {
        return failure("project_staging_create_failed", error.message());
    }

    const auto rollback = [&temporary_root] {
        std::error_code ignored;
        std::filesystem::remove_all(temporary_root, ignored);
    };

    auto manifest = generated.project;
    manifest["_urpg_format_version"] = "1.0";
    manifest["_engine_version_min"] = "0.1.0";
    manifest["determinism"] = {
        {"level", "A"}, {"authoritative_math", "fixed32"}, {"physics", {{"authoritative", true}, {"note", "native"}}},
    };
    manifest["startup"] = {{"map", request.starter_map}, {"headless_safe", true}};
    manifest["creator"] = {{"display_preset", request.display_preset},
                           {"input_preset", request.input_preset},
                           {"starter_map", request.starter_map}};
    if (request.include_creator_vertical_slice_seed) {
        manifest["creator"]["vertical_slice_seed"] = "lantern_of_the_willow_draft";
    }

    const nlohmann::json starter_map = {{"schema", "urpg.map.v1"}, {"id", request.starter_map},
                                        {"width", 16}, {"height", 12}, {"spawn", {{"x", 4}, {"y", 6}}}};
    const nlohmann::json database = {{"schema", "urpg.database.v1"}, {"actors", nlohmann::json::array()},
                                     {"items", nlohmann::json::array()}, {"switches", nlohmann::json::array()}};
    const nlohmann::json save_profile = {{"schema", "urpg.save_profile.v1"}, {"slot_count", 3}, {"autosave", true}};
    const nlohmann::json input = {{"schema", "urpg.input.v1"}, {"preset", request.input_preset}};
    const nlohmann::json vertical_slice_map = {
        {"schema", "urpg.map.v1"},
        {"id", "moonwell_shrine"},
        {"width", 16},
        {"height", 12},
        {"spawn", {{"x", 3}, {"y", 5}}},
        {"links", nlohmann::json::array({{{"target_map", request.starter_map}, {"event_id", "return_to_elder"}}})},
    };
    const nlohmann::json vertical_slice_seed = {
        {"schema", "urpg.creator_vertical_slice_seed.v1"},
        {"id", "lantern_of_the_willow"},
        {"status", "draft"},
        {"seed_revision", "native_creator_seed.v2"},
        {"maps", {request.starter_map, "moonwell_shrine"}},
        {"player", "willow_hero"},
        {"npcs", {"elder_mira", "vendor_rowan"}},
        {"quest", "restore_moonwell_lantern"},
        {"required_creator_routes",
         {"event_authoring", "message_inspector", "character_creator", "database", "quest", "battle_preview",
          "ability", "vendor", "audio_mix", "accessibility", "input_remap", "export_diagnostics"}},
        {"completion_note", "This seed is intentionally incomplete until every listed route saves, runs, and packages through the native creator workflow."},
    };

    // The creator seed is a real set of native document drafts, not a checklist
    // pointing authors to hand-written JSON. It remains a draft until its own
    // creator controls have produced runtime and package evidence.
    const auto make_perspective_draft = [](const std::string& map_id, nlohmann::json events) {
        return nlohmann::json{
            {"document_kind", "urpg.perspective_2d.map"},
            {"version", 1},
            {"map_id", map_id},
            {"width", 16},
            {"height", 12},
            {"selected_layer_id", "events"},
            {"selected_palette_option_id", ""},
            {"selected_tileset_id", ""},
            {"selected_tile_id", ""},
            {"layers", nlohmann::json::array({{{"id", "events"}, {"label", "Events"}, {"kind", "event"},
                                                 {"visible", true}, {"locked", false}, {"order", 0}}})},
            {"tiles", nlohmann::json::array()},
            {"events", std::move(events)},
            {"tileset_pages", nlohmann::json::array()},
            {"tile_definitions", nlohmann::json::array()},
            {"tile_palette", nlohmann::json::array()},
            {"prop_palette", nlohmann::json::array()},
            {"project_database", nlohmann::json::object()},
        };
    };
    const auto make_event = [](std::string id, std::string label, int x, int y, nlohmann::json commands) {
        return nlohmann::json{{"event_id", std::move(id)},
                              {"label", std::move(label)},
                              {"trigger_id", "confirm_interact"},
                              {"layer_id", "events"},
                              {"x", x},
                              {"y", y},
                              {"selected_page_id", "page_1"},
                              {"commands", nlohmann::json::array()},
                              {"pages", nlohmann::json::array({{{"page_id", "page_1"},
                                                                 {"label", "Page 1"},
                                                                 {"trigger_id", "confirm_interact"},
                                                                 {"order", 0},
                                                                 {"conditions", nlohmann::json::array()},
                                                                 {"commands", std::move(commands)}}})}};
    };
    const auto village_perspective_draft = make_perspective_draft(
        request.starter_map,
        nlohmann::json::array({
            make_event("elder_mira_intro", "Elder Mira", 5, 5,
                       nlohmann::json::array({{{"code", "show_text"},
                                                {"argument", "The moonwell lantern has gone dark."}},
                                               {{"code", "show_choice"}, {"argument", "accept_lantern_quest"}},
                                               {{"code", "change_switch"}, {"argument", "lantern_quest_started=true"}}})),
            make_event("vendor_rowan", "Rowan's Tonics", 7, 5,
                       nlohmann::json::array({{{"code", "open_vendor"}, {"argument", "rowan_tonics"}}})),
            make_event("walk_to_shrine", "Path to Moonwell Shrine", 14, 6,
                       nlohmann::json::array({{{"code", "transfer_player"}, {"argument", "moonwell_shrine:3,5"}}})),
        }));
    const auto shrine_perspective_draft = make_perspective_draft(
        "moonwell_shrine",
        nlohmann::json::array({
            make_event("shrine_wisp_encounter", "Shrine Wisp", 8, 5,
                       nlohmann::json::array({{{"code", "start_battle"}, {"argument", "shrine_wisp"}}})),
            make_event("recover_moonwell_lantern", "Moonwell Lantern", 10, 5,
                       nlohmann::json::array({{{"code", "change_item"}, {"argument", "moonwell_lantern:1"}},
                                               {{"code", "change_switch"}, {"argument", "lantern_recovered=true"}}})),
            make_event("return_to_elder", "Return to Elder Mira", 2, 5,
                       nlohmann::json::array({{{"code", "transfer_player"}, {"argument", "willow_village:4,6"}}})),
        }));
    const nlohmann::json vertical_slice_character = {
        {"schemaVersion", "1.0.0"}, {"name", "Willow Hero"}, {"portraitId", "portrait_ranger_01"},
        {"bodySpriteId", "sprite_ranger_body"}, {"portraitAssetId", ""}, {"fieldSpriteAssetId", ""},
        {"battleSpriteAssetId", ""}, {"classId", "class_ranger"}, {"speciesId", ""}, {"originId", ""},
        {"backgroundId", ""}, {"baseAttributes", {{"Attack", 12.0f}, {"Defense", 8.0f}}},
        {"appearanceTokens", nlohmann::json::array()}, {"layeredPartAssetIds", nlohmann::json::array()},
    };
    const nlohmann::json vertical_slice_quest = {
        {"schema_version", "urpg.quest_objective_graph.v1"},
        {"quest_id", "restore_moonwell_lantern"},
        {"title", "Restore the Moonwell Lantern"},
        {"nodes", nlohmann::json::array({
                      {{"id", "start"}, {"type", "start"}, {"title", "Begin"}, {"objective_id", ""},
                       {"localization_key", ""}, {"conditions", nlohmann::json::array()}, {"rewards", nlohmann::json::array()}},
                      {{"id", "recover_lantern"}, {"type", "objective"}, {"title", "Recover the lantern"},
                       {"objective_id", "recover_moonwell_lantern"}, {"localization_key", ""},
                       {"conditions", nlohmann::json::array({{{"type", "item"}, {"id", "moonwell_lantern"}, {"value", 0}}})},
                       {"rewards", nlohmann::json::array({{{"type", "gold"}, {"id", "elder_mira"}, {"value", 100}}})}},
                      {{"id", "complete"}, {"type", "complete"}, {"title", "Complete"}, {"objective_id", ""},
                       {"localization_key", ""}, {"conditions", nlohmann::json::array()}, {"rewards", nlohmann::json::array()}},
                  })},
        {"links", nlohmann::json::array({{{"from", "start"}, {"to", "recover_lantern"}},
                                           {{"from", "recover_lantern"}, {"to", "complete"}}})},
    };
    const nlohmann::json vertical_slice_database = {
        {"schema", "urpg.database.v1"},
        {"actors", nlohmann::json::array({{{"id", "willow_hero"}, {"name", "Willow Hero"}, {"class_id", "ranger"},
                                              {"max_hp", 100}, {"attack", 12}}})},
        {"items", nlohmann::json::array({{{"id", "moonwell_lantern"}, {"name", "Moonwell Lantern"}, {"price", 75},
                                             {"tags", nlohmann::json::array({"quest", "vendor"})}}})},
    };
    const nlohmann::json vertical_slice_vendor = {
        {"schema", "urpg.vendor_catalog.v1"},
        {"vendors", nlohmann::json::array({{{"id", "rowan_tonics"},
                                               {"stock", nlohmann::json::array({{{"item_id", "moonwell_lantern"},
                                                                                   {"quantity", 1}, {"buy_price", 75},
                                                                                   {"sell_price", 35},
                                                                                   {"required_flags", nlohmann::json::array()}}})}}})},
    };
    const nlohmann::json vertical_slice_ability = {
        {"ability_id", "willow_strike"}, {"cooldown_seconds", 3.0f}, {"mp_cost", 5.0f},
        {"effect_id", "willow_strike.damage"}, {"effect_attribute", "Attack"}, {"effect_operation", "Add"},
        {"effect_value", 10.0f}, {"effect_duration", 0.0f}, {"active_condition", ""}, {"passive_condition", ""},
        {"pattern", {{"name", "Willow Strike"}, {"points", nlohmann::json::array({{{"x", 0}, {"y", 0}}})}}},
    };
    if (!writeJson(temporary_root / "project.json", manifest) ||
        !writeJson(temporary_root / "content" / "maps" / (request.starter_map + ".json"), starter_map) ||
        !writeJson(temporary_root / "content" / "database.json", database) ||
        !writeJson(temporary_root / "content" / "save_profile.json", save_profile) ||
        !writeJson(temporary_root / "config" / "input_mappings.json", input) ||
        (request.include_creator_vertical_slice_seed &&
         (!writeJson(temporary_root / "content" / "maps" / "moonwell_shrine.json", vertical_slice_map) ||
          !writeJson(temporary_root / "content" / "maps" / (request.starter_map + ".p2d.json"), village_perspective_draft) ||
          !writeJson(temporary_root / "content" / "maps" / "moonwell_shrine.p2d.json", shrine_perspective_draft) ||
          !writeJson(temporary_root / "content" / "characters" / "willow_hero.json", vertical_slice_character) ||
          !writeJson(temporary_root / "content" / "quests" / "restore_moonwell_lantern.json", vertical_slice_quest) ||
          !writeJson(temporary_root / "content" / "vendors" / "rowan_tonics.json", vertical_slice_vendor) ||
          !writeJson(temporary_root / "content" / "abilities" / "willow_strike.json", vertical_slice_ability) ||
          !writeJson(temporary_root / "content" / "creator_vertical_slice_seed.json", vertical_slice_seed) ||
          !writeJson(temporary_root / "content" / "database.json", vertical_slice_database)))) {
        rollback();
        return failure("project_staging_write_failed", "Unable to write required starter-project files.");
    }

    if (const auto preflight = diagnostics::validateRuntimeProjectPreflight("editor", temporary_root, true)) {
        rollback();
        return failure("project_preflight_failed", preflight->code + ": " + preflight->message);
    }

#ifdef _WIN32
    if (!MoveFileExW(temporary_root.c_str(), request.destination.c_str(), MOVEFILE_WRITE_THROUGH)) {
        rollback();
        return failure("project_publish_failed", "Unable to publish the staged project directory.");
    }
#else
    std::filesystem::rename(temporary_root, request.destination, error);
    if (error) {
        rollback();
        return failure("project_publish_failed", error.message());
    }
#endif

    ProjectCreationResult result;
    result.success = true;
    result.code = "project_created";
    result.message = "Starter project was created and passed runtime preflight.";
    result.project_root = request.destination;
    result.audit_report = generated.audit_report;
    result.audit_report["runtime_preflight"] = "passed";
    return result;
}

} // namespace urpg::project
