#pragma once

#include "engine/core/asset_compressor.h"
#include "engine/core/security/sha256.h"

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
     * URPG-RLE compression plus XOR obfuscation. This is not an advanced
     * content-protection pipeline.
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
         * @brief Obfuscate a byte buffer with the current reversible XOR layer.
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
         * @brief Compute HMAC-SHA256 for bundle-level authenticity checks.
         * This is a symmetric verification boundary, not public-key code signing.
         */
        [[nodiscard]] std::string computeCryptographicSignature(std::string_view scope,
                                                                const std::vector<uint8_t>& data,
                                                                const std::string& key) const {
            constexpr std::size_t kBlockSize = 64u;
            std::vector<std::uint8_t> keyBytes(key.begin(), key.end());
            if (keyBytes.size() > kBlockSize) {
                const auto digest = Sha256::compute(keyBytes);
                keyBytes.assign(digest.begin(), digest.end());
            }
            keyBytes.resize(kBlockSize, 0u);

            std::vector<std::uint8_t> innerPad(kBlockSize, 0x36u);
            std::vector<std::uint8_t> outerPad(kBlockSize, 0x5cu);
            for (std::size_t i = 0; i < kBlockSize; ++i) {
                innerPad[i] ^= keyBytes[i];
                outerPad[i] ^= keyBytes[i];
            }

            std::vector<std::uint8_t> message;
            message.reserve(scope.size() + data.size() + 1u);
            for (const auto ch : scope) {
                message.push_back(static_cast<std::uint8_t>(ch));
            }
            message.push_back(0xffu);

            message.insert(message.end(), data.begin(), data.end());

            std::vector<std::uint8_t> inner;
            inner.reserve(innerPad.size() + message.size());
            inner.insert(inner.end(), innerPad.begin(), innerPad.end());
            inner.insert(inner.end(), message.begin(), message.end());
            const auto innerDigest = Sha256::compute(inner);

            std::vector<std::uint8_t> outer;
            outer.reserve(outerPad.size() + innerDigest.size());
            outer.insert(outer.end(), outerPad.begin(), outerPad.end());
            outer.insert(outer.end(), innerDigest.begin(), innerDigest.end());
            return Sha256::toHex(Sha256::compute(outer));
        }
    };

} // namespace urpg::security
