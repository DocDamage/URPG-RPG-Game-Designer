#include "sprite_pipeline_defs.h"
#include <iostream>
#include <fstream>
#include <filesystem>

/**
 * @brief Simple Sprite Pipeline CLI for atlas metadata generation and trimming.
 * 
 * Usage: urpg_sprite_pack --input <dir> --output <json>
 */

using namespace urpg::tools;
namespace fs = std::filesystem;

void printUsage() {
    std::cout << "URPG Sprite Pipeline CLI v1.0\n";
    std::cout << "Usage: urpg_sprite_pack --input <source_dir> --output <metadata_json>\n";
}

int main(int argc, char* argv[]) {
    std::string inputDir;
    std::string outputFile;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--input" && i + 1 < argc) {
            inputDir = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputFile = argv[++i];
        }
    }

    if (inputDir.empty() || outputFile.empty()) {
        printUsage();
        return 1;
    }

    std::cout << "Processing sprites from: " << inputDir << "...\n";

    SpriteAtlas atlas;
    atlas.atlasName = fs::path(outputFile).stem().string();
    atlas.texturePath = atlas.atlasName + ".png";
    atlas.width = 1024; // Default baseline
    atlas.height = 1024;

    // Simulate scanning directory for PNGs
    // In a real implementation, we would use an image library to get dimensions/trim
    if (fs::exists(inputDir) && fs::is_directory(inputDir)) {
        int currentX = 0;
        int currentY = 0;
        int maxHeightInRow = 0;

        for (const auto& entry : fs::directory_iterator(inputDir)) {
            if (entry.path().extension() == ".png") {
                SpriteRect rect;
                rect.id = entry.path().stem().string();
                rect.width = 64;  // Simulated size
                rect.height = 64; // Simulated size
                
                // Simple shelf packing logic
                if (currentX + rect.width > atlas.width) {
                    currentX = 0;
                    currentY += maxHeightInRow;
                    maxHeightInRow = 0;
                }

                rect.x = currentX;
                rect.y = currentY;
                rect.pivotX = rect.width / 2;
                rect.pivotY = rect.height / 2;

                atlas.sprites.push_back(rect);

                currentX += rect.width;
                if (rect.height > maxHeightInRow) maxHeightInRow = rect.height;
            }
        }
    }

    populatePreviewArtifacts(atlas);

    std::ofstream out(outputFile);
    out << toJson(atlas).dump(4);
    out.close();

    std::cout << "Successfully packed " << atlas.sprites.size() << " sprites into " << outputFile << "\n";

    return 0;
}
