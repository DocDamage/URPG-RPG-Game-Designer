#include "engine/core/audio/audio_core.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

namespace {

struct Options {
    std::filesystem::path projectRoot = std::filesystem::current_path();
    std::string asset;
    int durationMs = 1500;
};

void printUsage() {
    std::cout
        << "Usage: urpg_audio_smoke --asset <wav path or asset id> [--project-root <path>] [--duration-ms <ms>]\n";
}

Options parseOptions(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--asset" && i + 1 < argc) {
            options.asset = argv[++i];
        } else if (arg == "--project-root" && i + 1 < argc) {
            options.projectRoot = argv[++i];
        } else if (arg == "--duration-ms" && i + 1 < argc) {
            options.durationMs = std::max(1, std::stoi(argv[++i]));
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            std::exit(0);
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv) {
    const Options options = parseOptions(argc, argv);
    if (options.asset.empty()) {
        printUsage();
        return 2;
    }

    urpg::audio::AudioCore audio;
    audio.setAssetRoot(options.projectRoot);
    audio.clearAudioDiagnostics();

    const auto handle = audio.playSound(options.asset, urpg::audio::AudioCategory::SE);
    std::this_thread::sleep_for(std::chrono::milliseconds(options.durationMs));

    const auto diagnostics = audio.audioDiagnostics();
    if (!diagnostics.empty()) {
        for (const auto& diagnostic : diagnostics) {
            std::cerr << diagnostic.code << ": " << diagnostic.message;
            if (!diagnostic.asset_id.empty()) {
                std::cerr << " (" << diagnostic.asset_id << ")";
            }
            std::cerr << "\n";
        }
        audio.stopHandle(handle);
        return 1;
    }

    audio.stopHandle(handle);
    std::cout << "Audio smoke playback completed for " << options.asset << "\n";
    return 0;
}
