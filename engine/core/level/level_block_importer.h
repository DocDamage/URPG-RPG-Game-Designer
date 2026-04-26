#pragma once

#include "level_assembly.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

namespace urpg::level {

using json = nlohmann::json;

class LevelBlockImporter {
  public:
    static LevelBlockLibrary importLibraryDefinition(const std::string& filePath) {
        LevelBlockLibrary library;
        std::ifstream file(filePath);
        if (!file.is_open())
            return library;

        json data;
        file >> data;

        if (data.contains("libraryName") && data["libraryName"].is_string()) {
            library.setName(data["libraryName"].get<std::string>());
        }

        if (data.contains("blocks") && data["blocks"].is_array()) {
            for (const auto& item : data["blocks"]) {
                LevelBlock block(item["id"]);
                if (item.contains("prefabPath") && item["prefabPath"].is_string()) {
                    block.setPrefabPath(item["prefabPath"].get<std::string>());
                }

                if (item.contains("connectors") && item["connectors"].is_array()) {
                    for (const auto& connData : item["connectors"]) {
                        SnapConnector connector;
                        connector.type = connData["type"];
                        connector.side = stringToSide(connData["side"]);

                        if (connData.contains("offset") && connData["offset"].is_array() &&
                            connData["offset"].size() >= 3) {
                            connector.localX = connData["offset"][0];
                            connector.localY = connData["offset"][1];
                            connector.localZ = connData["offset"][2];
                        } else {
                            connector.localX = connector.localY = connector.localZ = 0.0f;
                        }

                        block.addConnector(connector);
                    }
                }
                library.addBlock(block);
            }
        }

        return library;
    }

    static std::vector<LevelBlock> importLibrary(const std::string& filePath) {
        return importLibraryDefinition(filePath).getBlocks();
    }

  private:
    static ConnectorSide stringToSide(const std::string& sideStr) {
        if (sideStr == "North")
            return ConnectorSide::North;
        if (sideStr == "South")
            return ConnectorSide::South;
        if (sideStr == "East")
            return ConnectorSide::East;
        if (sideStr == "West")
            return ConnectorSide::West;
        if (sideStr == "Up")
            return ConnectorSide::Up;
        if (sideStr == "Down")
            return ConnectorSide::Down;
        return ConnectorSide::North; // Default fallback
    }
};

} // namespace urpg::level
