#pragma once

#include "engine/core/asset_compressor.h"

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <cstdint>
#include <string_view>

namespace urpg::security {

    /**
     * @brief Resource protection helpers for the current export/runtime boundary.
     * The shipped in-tree contract is lightweight real protection only:
     * URPG-RLE compression plus XOR obfuscation. This is not a full encrypted
     * or shipping-hardened content pipeline.
     * Part of Wave 4 Engine Polish (4.6).
     */
    class ResourceProtector {
    public:
        /**
         * @brief Report whether real compression is implemented.
         */
        [[nodiscard]] static constexpr bool compressionImplemented() {
            return true;
        }

        /**
         * @brief Compress the input bytes using the in-tree URPG-RLE asset compressor.
         * This is lightweight real compression, not a no-op passthrough.
         */
        [[nodiscard]] std::vector<uint8_t> compress(const std::vector<uint8_t>& rawData) {
            return urpg::core::AssetCompressor::instance().compress(rawData);
        }

        /**
         * @brief Obfuscate a byte buffer (XOR or similar light-weight encryption).
         * Used to protect assets from simple rippers.
         */
        void obfuscate(std::vector<uint8_t>& data, const std::string& key) {
            urpg::core::AssetCompressor::instance().obfuscate(data, key);
        }

        /**
         * @brief Compute a lightweight keyed integrity tag for a protected payload.
         * This is a bounded tamper/corruption indicator, not cryptographic signing.
         */
        [[nodiscard]] std::string computeIntegrityTag(std::string_view scope,
                                                      const std::vector<uint8_t>& data,
                                                      const std::string& key) const {
            constexpr std::uint64_t kFnvOffset = 14695981039346656037ull;
            constexpr std::uint64_t kFnvPrime = 1099511628211ull;

            auto hash = kFnvOffset;
            const auto mixByte = [&hash](std::uint8_t byte) {
                hash ^= byte;
                hash *= kFnvPrime;
            };

            for (const auto ch : key) {
                mixByte(static_cast<std::uint8_t>(ch));
            }
            mixByte(0xff);

            for (const auto ch : scope) {
                mixByte(static_cast<std::uint8_t>(ch));
            }
            mixByte(0xfe);

            for (const auto byte : data) {
                mixByte(byte);
            }

            static constexpr char kHex[] = "0123456789abcdef";
            std::string encoded(16, '0');
            for (int i = 15; i >= 0; --i) {
                encoded[static_cast<std::size_t>(i)] = kHex[hash & 0x0f];
                hash >>= 4;
            }

            return encoded;
        }

        /**
         * @brief Script Logic Obfuscator (Stub).
         * Replaces readable symbol names with non-descript ones.
         */
        std::string obfuscateScript(const std::string& scriptSource) {
            std::string result = scriptSource;
            // Simplified: Represents a JS/Lua source-to-source obfuscator
            // This would normally involve a parser/transformer (e.g., Babel/Uglify)
            std::string logMsg = "/* [URPG Obfuscation Applied] */\n";
            return logMsg + result;
        }
    };

} // namespace urpg::security
