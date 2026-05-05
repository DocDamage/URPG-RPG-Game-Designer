#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::cli {

enum class CliAction {
    Run,
    Help,
    Version,
};

struct RuntimeCliOptions {
    bool headless = false;
    int frames = -1;
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    bool width_provided = false;
    bool height_provided = false;
    std::filesystem::path project_root = std::filesystem::current_path();
};

struct EditorCliOptions {
    bool headless = false;
    int frames = -1;
    std::uint32_t width = 1440;
    std::uint32_t height = 900;
    bool width_provided = false;
    bool height_provided = false;
    bool list_panels = false;
    bool render_all_panels = false;
    bool smoke = false;
    bool safe_mode = false;
    bool probe_platform = false;
    bool probe_opengl = false;
    bool probe_render = false;
    std::optional<std::string> open_panel_id;
    std::filesystem::path project_root = std::filesystem::current_path();
    std::filesystem::path smoke_output;
    std::filesystem::path smoke_snapshot_root;
};

struct RuntimeCliParseResult {
    RuntimeCliOptions options;
    CliAction action = CliAction::Run;
    std::string error;

    bool ok() const {
        return error.empty();
    }
};

struct EditorCliParseResult {
    EditorCliOptions options;
    CliAction action = CliAction::Run;
    std::string error;

    bool ok() const {
        return error.empty();
    }
};

std::vector<std::string_view> argvToViews(int argc, char** argv);

RuntimeCliParseResult parseRuntimeCli(std::vector<std::string_view> args, bool defaultHeadless);
EditorCliParseResult parseEditorCli(std::vector<std::string_view> args, bool defaultHeadless);

std::string runtimeHelpText();
std::string editorHelpText();

} // namespace urpg::cli
