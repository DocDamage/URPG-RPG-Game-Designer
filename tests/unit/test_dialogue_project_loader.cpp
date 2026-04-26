#include <catch2/catch_test_macros.hpp>

#include "engine/core/engine_shell.h"
#include "engine/core/message/dialogue_project_loader.h"
#include "engine/core/message/dialogue_registry.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path uniqueTempProjectRoot(const std::string& name) {
    const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() / ("urpg_" + name + "_" + std::to_string(unique));
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

void writeText(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    REQUIRE(out.is_open());
    out << text;
}

} // namespace

TEST_CASE("DialogueProjectLoader imports native dialogue sequence project data", "[message][dialogue_loader]") {
    auto& registry = urpg::message::DialogueRegistry::getInstance();
    registry.clear();

    const auto document = nlohmann::json{
        {"_urpg_format_version", "1.0"},
        {"sequences", nlohmann::json::array({
                          {
                              {"id", "intro"},
                              {"pages", nlohmann::json::array({
                                            {
                                                {"id", "p0"},
                                                {"style_id", "default"},
                                                {"presentation_mode", "speaker"},
                                                {"tone", "portrait"},
                                                {"speaker", "Guide"},
                                                {"face_actor_id", 2},
                                                {"body", "Welcome."},
                                                {"wait_for_advance", true},
                                                {"command", "GIVE_ITEM:potion"},
                                                {"choices", nlohmann::json::array({
                                                                {{"id", "ok"}, {"label", "OK"}, {"enabled", true}},
                                                            })},
                                            },
                                            {
                                                {"id", "p1"},
                                                {"style_id", "default"},
                                                {"presentation_mode", "narration"},
                                                {"tone", "neutral"},
                                                {"body", "The path opens."},
                                                {"wait_for_advance", true},
                                            },
                                        })},
                          },
                      })},
    };

    const auto result =
        urpg::message::DialogueProjectLoader::importDocument(registry, document, "memory/dialogue_sequences.json");
    REQUIRE(result.loaded);
    REQUIRE(result.conversation_count == 1);
    REQUIRE(result.node_count == 2);
    REQUIRE_FALSE(result.diagnostics.empty());

    const auto* conversation = registry.getConversation("intro");
    REQUIRE(conversation != nullptr);
    REQUIRE(conversation->size() == 2);
    REQUIRE((*conversation)[0].variant.speaker == "Guide");
    REQUIRE((*conversation)[0].variant.face_actor_id == 2);
    REQUIRE((*conversation)[0].command == "GIVE_ITEM:potion");
    REQUIRE((*conversation)[0].next_node_id == "p1");
    REQUIRE((*conversation)[0].choices.size() == 1);
}

TEST_CASE("DialogueProjectLoader reports missing project dialogue content", "[message][dialogue_loader]") {
    auto& registry = urpg::message::DialogueRegistry::getInstance();
    registry.clear();

    const auto projectRoot = uniqueTempProjectRoot("missing_dialogue");
    const auto result = urpg::message::DialogueProjectLoader::loadProject(registry, projectRoot);

    REQUIRE_FALSE(result.loaded);
    REQUIRE(result.conversation_count == 0);
    REQUIRE(result.diagnostics.size() == 1);
    REQUIRE(result.diagnostics[0].code == "dialogue_content_missing");
    REQUIRE(registry.getConversations().empty());

    std::filesystem::remove_all(projectRoot);
}

TEST_CASE("EngineShell startup loads dialogue from project content without mock fallback",
          "[message][engine_shell][dialogue_loader]") {
    auto& shell = urpg::EngineShell::getInstance();
    if (shell.isRunning()) {
        shell.shutdown();
    }

    const auto projectRoot = uniqueTempProjectRoot("shell_dialogue");
    writeText(projectRoot / "content" / "dialogue_sequences.json",
              R"({
                "_urpg_format_version": "1.0",
                "sequences": [
                  {
                    "id": "project_intro",
                    "pages": [
                      {
                        "id": "start",
                        "style_id": "default",
                        "presentation_mode": "speaker",
                        "tone": "portrait",
                        "speaker": "Project NPC",
                        "body": "Loaded from project content.",
                        "wait_for_advance": true
                      }
                    ]
                  }
                ]
              })");

    REQUIRE(shell.startup(std::make_unique<urpg::HeadlessSurface>(), std::make_unique<urpg::HeadlessRenderer>(),
                          urpg::EngineShell::StartupOptions(projectRoot)));

    const auto metadata = shell.exportDialogueMetadata();
    REQUIRE(metadata.size() == 1);
    REQUIRE(metadata[0]["id"] == "project_intro");
    REQUIRE(shell.getDialogueLoadResult().loaded);
    REQUIRE(shell.exportDialogueLoadDiagnostics()[0]["code"] == "dialogue_content_loaded");

    shell.shutdown();
    std::filesystem::remove_all(projectRoot);
}

TEST_CASE("EngineShell startup does not seed mock dialogue when project content is absent",
          "[message][engine_shell][dialogue_loader]") {
    auto& shell = urpg::EngineShell::getInstance();
    if (shell.isRunning()) {
        shell.shutdown();
    }

    const auto projectRoot = uniqueTempProjectRoot("shell_no_dialogue");
    REQUIRE(shell.startup(std::make_unique<urpg::HeadlessSurface>(), std::make_unique<urpg::HeadlessRenderer>(),
                          urpg::EngineShell::StartupOptions(projectRoot)));

    REQUIRE(shell.exportDialogueMetadata().empty());
    const auto diagnostics = shell.exportDialogueLoadDiagnostics();
    REQUIRE(diagnostics.size() == 1);
    REQUIRE(diagnostics[0]["code"] == "dialogue_content_missing");

    shell.shutdown();
    std::filesystem::remove_all(projectRoot);
}
