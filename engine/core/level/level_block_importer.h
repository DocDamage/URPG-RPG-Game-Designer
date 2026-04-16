#pragma once

#include "level_assembly.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

namespace urpg::level {

using json = nlohmann::json;

class LevelBlockImporter {
public:
    static std::vector<LevelBlock> importLibrary(const std::string& filePath) {
        std::vector<LevelBlock> blocks;
        std::ifstream file(filePath);
        if (!file.is_open()) return blocks;

        json data;
        file >> data;

        if (data.contains("blocks") && data["blocks"].is_array()) {
            for (const auto& item : data["blocks"]) {
                LevelBlock block(item["id"]);
                
                if (item.contains("connectors") && item["connectors"].is_array()) {
                    for (const auto& connData : item["connectors"]) {
                        SnapConnector connector;
                        connector.type = connData["type"];
                        connector.side = stringToSide(connData["side"]);
                        
                        if (connData.contains("offset") && connData["offset"].is_array() && connData["offset"].size() >= 3) {
                            connector.localX = connData["offset"][0];
                            connector.localY = connData["offset"][1];
                            connector.localZ = connData["offset"][2];
                        } else {
                            connector.localX = connector.localY = connector.localZ = 0.0f;
                        }

                        block.addConnector(connector);
                    }
                }
                blocks.push_back(block);
            }
        }

        return blocks;
    }

private:
    static ConnectorSide stringToSide(const std::string& sideStr) {
        if (sideStr == "North") return ConnectorSide::North;
        if (sideStr == "South") return ConnectorSide::South;
        if (sideStr == "East")  return ConnectorSide::East;
        if (sideStr == "West")  return ConnectorSide::West;
        if (sideStr == "Up")    return ConnectorSide::Up;
        if (sideStr == "Down")  return ConnectorSide::Down;
        return ConnectorSide::North; // Default fallback
    }
};

} // namespace urpg::level
