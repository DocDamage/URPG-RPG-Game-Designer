#include "asset_compressor.h"
#include <iostream>
#include <algorithm>

namespace urpg::core {

std::vector<uint8_t> AssetCompressor::compress(const std::vector<uint8_t>& rawData) {
    if (rawData.empty()) return {};

    std::vector<uint8_t> result;
    for (size_t i = 0; i < rawData.size(); ) {
        uint8_t val = rawData[i];
        uint8_t count = 1;
        
        while (i + count < rawData.size() && rawData[i + count] == val && count < 255) {
            count++;
        }

        result.push_back(count);
        result.push_back(val);
        i += count;
    }

    return result;
}

std::vector<uint8_t> AssetCompressor::decompress(const std::vector<uint8_t>& data) {
    if (data.size() % 2 != 0) return {};

    std::vector<uint8_t> result;
    for (size_t i = 0; i < data.size(); i += 2) {
        uint8_t count = data[i];
        uint8_t val = data[i + 1];
        result.insert(result.end(), count, val);
    }

    return result;
}

void AssetCompressor::obfuscate(std::vector<uint8_t>& data, const std::string& key) {
    if (key.empty()) return;
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= static_cast<uint8_t>(key[i % key.length()]);
    }
}

} // namespace urpg::core
