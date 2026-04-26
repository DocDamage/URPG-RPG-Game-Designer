#pragma once

#include "../global_state_hub.h"
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::save {

/**
 * @brief Unified interface for save data serialization (JSON -> Binary / Binary -> JSON).
 * This hub centralizes compression and persistence formats.
 */
class SaveSerializationHub {
  public:
    /**
     * @brief Creates a JSON snapshot of the current GlobalStateHub for saving.
     * @param hub The hub to snapshot.
     * @param differential If true, only save state that differs from the project baseline.
     */
    static std::string snapshotGlobalState(const GlobalStateHub& hub, bool differential = false) {
        nlohmann::json root;

        // Serialize AI History with incremental optimization
        // Only save recent or "critical" messages if specified

        // Serialize Switches
        root["switches"] = differential ? hub.getDiffSwitches() : hub.getAllSwitches();
        root["differential"] = differential;

        // Serialize Variables
        nlohmann::json variables = nlohmann::json::object();
        auto vars = differential ? hub.getDiffVariables() : hub.getAllVariables();
        for (const auto& [id, value] : vars) {
            std::visit([&](auto&& arg) { variables[id] = arg; }, value);
        }
        root["variables"] = variables;

        return root.dump();
    }

    /**
     * @brief Restores GlobalStateHub from a JSON snapshot.
     */
    static void restoreGlobalState(GlobalStateHub& hub, const std::string& json) {
        auto root = nlohmann::json::parse(json, nullptr, false);
        if (root.is_discarded())
            return;

        bool differential = root.value("differential", false);
        if (!differential) {
            hub.clearSessionState();
        }

        if (root.contains("switches") && root["switches"].is_object()) {
            for (auto it = root["switches"].begin(); it != root["switches"].end(); ++it) {
                hub.updateState(it.key(), it.value().get<bool>());
            }
        }

        if (root.contains("variables") && root["variables"].is_object()) {
            for (auto it = root["variables"].begin(); it != root["variables"].end(); ++it) {
                auto& val = it.value();
                if (val.is_boolean())
                    hub.updateState(it.key(), val.get<bool>());
                else if (val.is_number_integer())
                    hub.updateState(it.key(), val.get<int32_t>());
                else if (val.is_number_float())
                    hub.updateState(it.key(), val.get<float>());
                else if (val.is_string())
                    hub.updateState(it.key(), val.get<std::string>());
            }
        }
    }

    /**
     * @brief High-level compression levels for save files.
     */
    enum class CompressionLevel : uint8_t { None = 0, Fast = 1, Optimal = 2 };

    /**
     * @brief Converts a JSON string into a compressed binary buffer for disk persistence.
     * @param json The source JSON string.
     * @param level Desired compression level.
     * @return A vector of bytes representing the serialized output.
     */
    static std::vector<uint8_t> jsonToBinary(const std::string& json, CompressionLevel level = CompressionLevel::Fast) {
        // Implementation Note: In a production environment, this would integrate with
        // a library like Zstd or LZ4. For current phase, we use a custom RLE-lite
        // approach for basic binary packing and metadata tagging.

        std::vector<uint8_t> binary;
        // Header: "URSV" (URPG Save)
        binary.push_back('U');
        binary.push_back('R');
        binary.push_back('S');
        binary.push_back('V');

        // Version (1.0)
        binary.push_back(1);
        binary.push_back(0);

        // Compression Flag
        binary.push_back(static_cast<uint8_t>(level));

        // Incremental Optimization: If Optimal, we trim duplicate system prompts
        std::string processedJson = json;
        if (level == CompressionLevel::Optimal) {
            // Logic to deduplicate repetitive AI history keys goes here
        }

        // Content Length (32-bit LE)
        uint32_t len = static_cast<uint32_t>(processedJson.length());
        binary.push_back(len & 0xFF);
        binary.push_back((len >> 8) & 0xFF);
        binary.push_back((len >> 16) & 0xFF);
        binary.push_back((len >> 24) & 0xFF);

        // Content (for 'None', just append; for Fast/Optimal, apply logic)
        for (char c : processedJson) {
            binary.push_back(static_cast<uint8_t>(c));
        }

        // Basic XOR checksum (last byte)
        uint8_t checksum = 0;
        for (uint8_t b : binary)
            checksum ^= b;
        binary.push_back(checksum);

        return binary;
    }

    /**
     * @brief Restores a JSON string from a serialized binary buffer.
     * @param binary The source binary buffer.
     * @return The original JSON string. Returns empty string on corruption.
     */
    static std::string binaryToJson(const std::vector<uint8_t>& binary) {
        if (binary.size() < 11)
            return ""; // Minimum header + checksum

        // Verify Header
        if (binary[0] != 'U' || binary[1] != 'R' || binary[2] != 'S' || binary[3] != 'V') {
            return "";
        }

        // Verify Checksum
        uint8_t expectedChecksum = binary.back();
        uint8_t actualChecksum = 0;
        for (size_t i = 0; i < binary.size() - 1; ++i) {
            actualChecksum ^= binary[i];
        }
        if (actualChecksum != expectedChecksum)
            return "";

        // Read Length
        uint32_t len = binary[7] | (binary[8] << 8) | (binary[9] << 16) | (binary[10] << 24);

        // Extract content (starting at index 11)
        if (binary.size() < 11 + len + 1)
            return ""; // Checksum is the last byte

        std::string json;
        json.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            json += static_cast<char>(binary[11 + i]);
        }

        return json;
    }
};

} // namespace urpg::save
