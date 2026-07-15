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
        {"maps", {request.starter_map, "moonwell_shrine"}},
        {"player", "willow_hero"},
        {"npcs", {"elder_mira", "vendor_rowan"}},
        {"quest", "restore_moonwell_lantern"},
        {"required_creator_routes",
         {"event_authoring", "message_inspector", "character_creator", "database", "quest", "battle_preview",
          "ability", "vendor", "audio_mix", "accessibility", "input_remap", "export_diagnostics"}},
        {"completion_note", "This seed is intentionally incomplete until every listed route saves, runs, and packages through the native creator workflow."},
    };
    if (!writeJson(temporary_root / "project.json", manifest) ||
        !writeJson(temporary_root / "content" / "maps" / (request.starter_map + ".json"), starter_map) ||
        !writeJson(temporary_root / "content" / "database.json", database) ||
        !writeJson(temporary_root / "content" / "save_profile.json", save_profile) ||
        !writeJson(temporary_root / "config" / "input_mappings.json", input) ||
        (request.include_creator_vertical_slice_seed &&
         (!writeJson(temporary_root / "content" / "maps" / "moonwell_shrine.json", vertical_slice_map) ||
          !writeJson(temporary_root / "content" / "creator_vertical_slice_seed.json", vertical_slice_seed)))) {
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
