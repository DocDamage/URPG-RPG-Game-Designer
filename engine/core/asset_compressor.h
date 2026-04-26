#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::core {

/**
 * @brief Simple Run-Length Encoding and XOR-based obfuscation for initial protection.
 *
 * Part of the Post-Gold Asset Protection track.
 */
class AssetCompressor {
  public:
    static AssetCompressor& instance() {
        static AssetCompressor inst;
        return inst;
    }

    /**
     * @brief Compresses a raw byte buffer using a lightweight URPG-RLE algorithm.
     */
    std::vector<uint8_t> compress(const std::vector<uint8_t>& rawData);

    /**
     * @brief Decompresses an RLE buffer back to original data.
     */
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressedData);

    /**
     * @brief Applies an XOR-obfuscation pass to prevent simple header sniffing.
     */
    void obfuscate(std::vector<uint8_t>& data, const std::string& key);

  private:
    AssetCompressor() = default;
};

} // namespace urpg::core
