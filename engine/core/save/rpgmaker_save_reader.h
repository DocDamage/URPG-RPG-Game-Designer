#pragma once

#include "engine/core/save/lzstring_decompressor.h"
#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::save {

/**
 * @brief Detected format of an RPG Maker save file.
 */
enum class RPGMakerSaveFormat {
    Unknown,
    RPGMakerMV,   ///< LZString-compressed JSON with optional XOR encryption
    RPGMakerMZ,   ///< Same format as MV but with different data schema
};

/**
 * @brief Result of reading and parsing an RPG Maker .rpgsave file.
 */
struct RPGMakerSaveReadResult {
    bool success = false;
    RPGMakerSaveFormat format = RPGMakerSaveFormat::Unknown;
    nlohmann::json data;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

/**
 * @brief Reader for RPG Maker MV/MZ save files (.rpgsave).
 *
 * Handles format detection, optional XOR decryption, and LZString decompression.
 */
class RPGMakerSaveFileReader {
public:
    /**
     * @brief Read and parse a .rpgsave file from disk.
     *
     * @param filePath Path to the .rpgsave file.
     * @param encryptionKey Optional XOR encryption key (empty = no decryption).
     * @return Parse result with JSON data or error diagnostics.
     */
    static RPGMakerSaveReadResult readFile(const std::string& filePath,
                                           const std::string& encryptionKey = "");

    /**
     * @brief Detect the RPG Maker save format from raw file bytes.
     */
    static RPGMakerSaveFormat detectFormat(const std::vector<uint8_t>& bytes);

    /**
     * @brief Apply XOR decryption to encrypted save data.
     */
    static std::vector<uint8_t> decryptXOR(const std::vector<uint8_t>& data,
                                           const std::string& key);

private:
    static std::string trimNullPadding(const std::string& str);
};

} // namespace urpg::save
