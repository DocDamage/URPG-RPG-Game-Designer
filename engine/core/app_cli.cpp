#include "engine/core/app_cli.h"

#include <charconv>
#include <limits>

namespace urpg::cli {

namespace {

bool needsValue(std::string_view argument) {
    return argument.empty() || argument.rfind("--", 0) == 0;
}

bool parseInt(std::string_view text, int& value) {
    int parsed = 0;
    const auto* begin = text.data();
    const auto* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end) {
        return false;
    }
    value = parsed;
    return true;
}

bool parseUint32(std::string_view text, std::uint32_t& value) {
    unsigned long long parsed = 0;
    const auto* begin = text.data();
    const auto* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end || parsed > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }
    value = static_cast<std::uint32_t>(parsed);
    return true;
}

std::string missingValueError(std::string_view flag) {
    return "missing value after " + std::string(flag);
}

std::string invalidValueError(std::string_view flag, std::string_view value) {
    return "invalid value for " + std::string(flag) + ": " + std::string(value);
}

} // namespace

std::vector<std::string_view> argvToViews(int argc, char** argv) {
    std::vector<std::string_view> args;
    args.reserve(argc > 0 ? static_cast<std::size_t>(argc - 1) : 0);
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    return args;
}

RuntimeCliParseResult parseRuntimeCli(std::vector<std::string_view> args, bool defaultHeadless) {
    RuntimeCliParseResult result;
    result.options.headless = defaultHeadless;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "--help" || arg == "-h") {
            result.action = CliAction::Help;
        } else if (arg == "--version") {
            result.action = CliAction::Version;
        } else if (arg == "--headless") {
            result.options.headless = true;
        } else if (arg == "--frames") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            if (!parseInt(args[++i], result.options.frames)) {
                result.error = invalidValueError(arg, args[i]);
                return result;
            }
        } else if (arg == "--width") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            if (!parseUint32(args[++i], result.options.width)) {
                result.error = invalidValueError(arg, args[i]);
                return result;
            }
            result.options.width_provided = true;
        } else if (arg == "--height") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            if (!parseUint32(args[++i], result.options.height)) {
                result.error = invalidValueError(arg, args[i]);
                return result;
            }
            result.options.height_provided = true;
        } else if (arg == "--project-root") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            result.options.project_root = std::filesystem::path(std::string(args[++i]));
        } else {
            result.error = "unknown option: " + std::string(arg);
            return result;
        }
    }

    return result;
}

EditorCliParseResult parseEditorCli(std::vector<std::string_view> args, bool defaultHeadless) {
    EditorCliParseResult result;
    result.options.headless = defaultHeadless;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "--help" || arg == "-h") {
            result.action = CliAction::Help;
        } else if (arg == "--version") {
            result.action = CliAction::Version;
        } else if (arg == "--headless") {
            result.options.headless = true;
        } else if (arg == "--frames") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            if (!parseInt(args[++i], result.options.frames)) {
                result.error = invalidValueError(arg, args[i]);
                return result;
            }
        } else if (arg == "--width") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            if (!parseUint32(args[++i], result.options.width)) {
                result.error = invalidValueError(arg, args[i]);
                return result;
            }
            result.options.width_provided = true;
        } else if (arg == "--height") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            if (!parseUint32(args[++i], result.options.height)) {
                result.error = invalidValueError(arg, args[i]);
                return result;
            }
            result.options.height_provided = true;
        } else if (arg == "--list-panels") {
            result.options.list_panels = true;
        } else if (arg == "--render-all-panels") {
            result.options.render_all_panels = true;
        } else if (arg == "--smoke") {
            result.options.smoke = true;
            result.options.headless = true;
        } else if (arg == "--safe-mode") {
            result.options.safe_mode = true;
            result.options.headless = true;
        } else if (arg == "--probe-platform") {
            result.options.probe_platform = true;
        } else if (arg == "--probe-opengl") {
            result.options.probe_platform = true;
            result.options.probe_opengl = true;
        } else if (arg == "--open-panel") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            result.options.open_panel_id = std::string(args[++i]);
        } else if (arg == "--project-root") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            result.options.project_root = std::filesystem::path(std::string(args[++i]));
        } else if (arg == "--smoke-output") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            result.options.smoke_output = std::filesystem::path(std::string(args[++i]));
        } else if (arg == "--smoke-snapshot-root") {
            if (i + 1 >= args.size() || needsValue(args[i + 1])) {
                result.error = missingValueError(arg);
                return result;
            }
            result.options.smoke_snapshot_root = std::filesystem::path(std::string(args[++i]));
        } else {
            result.error = "unknown option: " + std::string(arg);
            return result;
        }
    }

    if (result.options.smoke) {
        if (result.options.frames < 0) {
            result.options.frames = 0;
        }
        if (result.options.smoke_output.empty()) {
            result.options.smoke_output = std::filesystem::temp_directory_path() / "urpg_editor_smoke_state.json";
        }
        if (result.options.smoke_snapshot_root.empty()) {
            result.options.smoke_snapshot_root = std::filesystem::temp_directory_path() / "urpg_editor_smoke_snapshots";
        }
    }
    if (result.options.safe_mode && result.options.frames < 0) {
        result.options.frames = 1;
    }

    return result;
}

std::string runtimeHelpText() {
    return "Usage: urpg_runtime [--headless] [--frames <count>] [--width <pixels>] [--height <pixels>] "
           "[--project-root <path>] [--version] [--help]\n";
}

std::string editorHelpText() {
    return "Usage: urpg_editor [--headless] [--frames <count>] [--width <pixels>] [--height <pixels>] "
           "[--project-root <path>] [--list-panels] [--render-all-panels] [--open-panel <id>] "
           "[--smoke] [--smoke-output <path>] [--smoke-snapshot-root <path>] [--safe-mode] "
           "[--probe-platform] [--probe-opengl] [--version] [--help]\n";
}

} // namespace urpg::cli
