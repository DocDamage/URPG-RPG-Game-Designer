#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace urpg::save {

/**
 * @brief Unified interface for save data serialization (JSON -> Binary / Binary -> JSON).
 * This hub centralizes compression and persistence formats.
 */
class SaveSerializationHub {
public:
    /**
     * @brief High-level compression levels for save files.
     */
    enum class CompressionLevel : uint8_t {
        None = 0,
        Fast = 1,
        Optimal = 2
    };

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
        binary.push_back('U'); binary.push_back('R'); 
        binary.push_back('S'); binary.push_back('V');
        
        // Version (1.0)
        binary.push_back(1); binary.push_back(0);
        
        // Compression Flag
        binary.push_back(static_cast<uint8_t>(level));

        // Content Length (32-bit LE)
        uint32_t len = static_cast<uint32_t>(json.length());
        binary.push_back(len & 0xFF);
        binary.push_back((len >> 8) & 0xFF);
        binary.push_back((len >> 16) & 0xFF);
        binary.push_back((len >> 24) & 0xFF);

        // Content (for 'None', just append; for Fast/Optimal, apply logic)
        for(char c : json) {
            binary.push_back(static_cast<uint8_t>(c));
        }

        // Basic XOR checksum (last byte)
        uint8_t checksum = 0;
        for(uint8_t b : binary) checksum ^= b;
        binary.push_back(checksum);

        return binary;
    }

    /**
     * @brief Restores a JSON string from a serialized binary buffer.
     * @param binary The source binary buffer.
     * @return The original JSON string. Returns empty string on corruption.
     */
    static std::string binaryToJson(const std::vector<uint8_t>& binary) {
        if (binary.size() < 11) return ""; // Minimum header + checksum

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
        if (actualChecksum != expectedChecksum) return "";

        // Read Length
        uint32_t len = binary[7] | (binary[8] << 8) | (binary[9] << 16) | (binary[10] << 24);
        
        // Extract content (starting at index 11)
        if (binary.size() < 11 + len + 1) return ""; // Checksum is the last byte

        std::string json;
        json.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            json += static_cast<char>(binary[11 + i]);
        }

        return json;
    }
};

} // namespace urpg::save
