#include "engine/core/platform/process_runner.h"
#include "engine/core/project/project_creation_service.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

namespace {

std::filesystem::path uniqueRoot() {
    return std::filesystem::temp_directory_path() /
           ("urpg_creator_runtime_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}

} // namespace

TEST_CASE("creator project starts the native runtime after atomic creation", "[integration][creator journey][project][runtime]") {
    const auto root = uniqueRoot();
    const auto destination = root / "FirstProject";
    urpg::project::ProjectCreationRequest request;
    request.project_id = "first_project";
    request.project_name = "First Project";
    request.destination = destination;

    const auto created = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE(created.success);

    urpg::platform::ProcessCommand command;
    command.executable = URPG_RUNTIME_PATH;
    command.arguments = {"--headless", "--frames", "1", "--project-root", destination.generic_string()};
    command.workingDirectory = destination;
    command.timeout = std::chrono::seconds(20);
    const auto launched = urpg::platform::runProcess(command);

    REQUIRE_FALSE(launched.timedOut);
    INFO(launched.stderrText);
    REQUIRE(launched.error.empty());
    REQUIRE(launched.exitCode == 0);
    REQUIRE(launched.stdoutText.find("URPG runtime exited after 1 frame") != std::string::npos);

    std::filesystem::remove_all(root);
}
