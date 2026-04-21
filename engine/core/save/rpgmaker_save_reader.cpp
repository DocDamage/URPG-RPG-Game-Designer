#include "engine/core/save/rpgmaker_save_reader.h"

#include <filesystem>
#include <fstream>

namespace urpg::save {

RPGMakerSaveFormat RPGMakerSaveFileReader::detectFormat(const std::vector<uint8_t>& bytes) {
    if (bytes.empty()) {
        return RPGMakerSaveFormat::Unknown;
    }

    // RPG Maker MV/MZ save files are LZString-compressed JSON.
    // The first bytes are Base64 characters (A-Z, a-z, 0-9, +, /, =).
    // We can't reliably distinguish MV vs MZ from raw bytes alone,
    // so we mark as MV by default and let the caller validate schema.
    bool looksLikeBase64 = true;
    size_t checkLen = std::min(size_t(16), bytes.size());
    for (size_t i = 0; i < checkLen; ++i) {
        char c = static_cast<char>(bytes[i]);
        bool valid = (c >= 'A' && c <= 'Z') ||
                     (c >= 'a' && c <= 'z') ||
                     (c >= '0' && c <= '9') ||
                     c == '+' || c == '/' || c == '=';
        if (!valid) {
            looksLikeBase64 = false;
            break;
        }
    }

    if (looksLikeBase64) {
        return RPGMakerSaveFormat::RPGMakerMV;
    }

    return RPGMakerSaveFormat::Unknown;
}

std::vector<uint8_t> RPGMakerSaveFileReader::decryptXOR(const std::vector<uint8_t>& data,
                                                        const std::string& key) {
    if (key.empty()) {
        return data;
    }

    std::vector<uint8_t> result = data;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] ^= static_cast<uint8_t>(key[i % key.size()]);
    }
    return result;
}

std::string RPGMakerSaveFileReader::trimNullPadding(const std::string& str) {
    auto pos = str.find_last_not_of('\0');
    if (pos == std::string::npos) {
        return "";
    }
    return str.substr(0, pos + 1);
}

RPGMakerSaveReadResult RPGMakerSaveFileReader::readFile(const std::string& filePath,
                                                        const std::string& encryptionKey) {
    RPGMakerSaveReadResult result;

    if (!std::filesystem::exists(filePath)) {
        result.errors.push_back("File not found: " + filePath);
        return result;
    }

    // Read raw bytes
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        result.errors.push_back("Failed to open file: " + filePath);
        return result;
    }

    std::vector<uint8_t> rawBytes((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
    file.close();

    if (rawBytes.empty()) {
        result.errors.push_back("File is empty: " + filePath);
        return result;
    }

    // Apply XOR decryption if key provided
    std::vector<uint8_t> decryptedBytes = rawBytes;
    if (!encryptionKey.empty()) {
        decryptedBytes = decryptXOR(rawBytes, encryptionKey);
        result.warnings.push_back("XOR decryption applied with provided key.");
    }

    // Detect format after decryption so encrypted Base64 payloads can be recognized.
    result.format = detectFormat(decryptedBytes);
    if (result.format == RPGMakerSaveFormat::Unknown) {
        result.errors.push_back("Unable to detect RPG Maker save format from file contents.");
        return result;
    }

    // Convert bytes to string for LZString decompression
    std::string compressedStr(decryptedBytes.begin(), decryptedBytes.end());
    compressedStr = trimNullPadding(compressedStr);

    if (compressedStr.empty()) {
        result.errors.push_back("Decrypted file contents are empty.");
        return result;
    }

    // Decompress LZString
    auto decompressed = lzstring::decompressFromBase64(compressedStr);
    if (!decompressed.has_value()) {
        result.errors.push_back("LZString decompression failed; file may be corrupt or use an unsupported format.");
        return result;
    }

    // Parse JSON
    try {
        result.data = nlohmann::json::parse(decompressed.value());
    } catch (const nlohmann::json::exception& e) {
        result.errors.push_back("JSON parse error: " + std::string(e.what()));
        return result;
    }

    result.success = true;
    return result;
}

} // namespace urpg::save
