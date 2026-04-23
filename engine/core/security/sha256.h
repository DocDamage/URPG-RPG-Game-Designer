#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace urpg::security {

/**
 * @brief Minimal in-tree SHA-256 helper for bundle-signing verification.
 */
class Sha256 {
public:
    static std::array<std::uint8_t, 32> compute(const std::vector<std::uint8_t>& data) {
        std::array<std::uint32_t, 8> state = {
            0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
            0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u,
        };

        auto message = data;
        const std::uint64_t bitLength = static_cast<std::uint64_t>(message.size()) * 8ull;
        message.push_back(0x80u);
        while ((message.size() % 64u) != 56u) {
            message.push_back(0x00u);
        }
        for (int shift = 56; shift >= 0; shift -= 8) {
            message.push_back(static_cast<std::uint8_t>((bitLength >> shift) & 0xffu));
        }

        for (std::size_t blockOffset = 0; blockOffset < message.size(); blockOffset += 64u) {
            std::uint32_t schedule[64] = {};
            for (std::size_t i = 0; i < 16u; ++i) {
                const std::size_t index = blockOffset + i * 4u;
                schedule[i] = (static_cast<std::uint32_t>(message[index]) << 24u) |
                              (static_cast<std::uint32_t>(message[index + 1u]) << 16u) |
                              (static_cast<std::uint32_t>(message[index + 2u]) << 8u) |
                              static_cast<std::uint32_t>(message[index + 3u]);
            }
            for (std::size_t i = 16u; i < 64u; ++i) {
                schedule[i] = smallSigma1(schedule[i - 2u]) +
                              schedule[i - 7u] +
                              smallSigma0(schedule[i - 15u]) +
                              schedule[i - 16u];
            }

            std::uint32_t a = state[0];
            std::uint32_t b = state[1];
            std::uint32_t c = state[2];
            std::uint32_t d = state[3];
            std::uint32_t e = state[4];
            std::uint32_t f = state[5];
            std::uint32_t g = state[6];
            std::uint32_t h = state[7];

            for (std::size_t i = 0; i < 64u; ++i) {
                const std::uint32_t temp1 = h + bigSigma1(e) + choose(e, f, g) + kRoundConstants[i] + schedule[i];
                const std::uint32_t temp2 = bigSigma0(a) + majority(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            state[0] += a;
            state[1] += b;
            state[2] += c;
            state[3] += d;
            state[4] += e;
            state[5] += f;
            state[6] += g;
            state[7] += h;
        }

        std::array<std::uint8_t, 32> digest = {};
        for (std::size_t i = 0; i < state.size(); ++i) {
            digest[i * 4u + 0u] = static_cast<std::uint8_t>((state[i] >> 24u) & 0xffu);
            digest[i * 4u + 1u] = static_cast<std::uint8_t>((state[i] >> 16u) & 0xffu);
            digest[i * 4u + 2u] = static_cast<std::uint8_t>((state[i] >> 8u) & 0xffu);
            digest[i * 4u + 3u] = static_cast<std::uint8_t>(state[i] & 0xffu);
        }

        return digest;
    }

    static std::string toHex(const std::array<std::uint8_t, 32>& digest) {
        static constexpr char kHex[] = "0123456789abcdef";
        std::string encoded;
        encoded.reserve(digest.size() * 2u);
        for (const auto byte : digest) {
            encoded.push_back(kHex[(byte >> 4u) & 0x0fu]);
            encoded.push_back(kHex[byte & 0x0fu]);
        }
        return encoded;
    }

private:
    static constexpr std::uint32_t rotateRight(std::uint32_t value, std::uint32_t count) {
        return (value >> count) | (value << (32u - count));
    }

    static constexpr std::uint32_t choose(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
        return (x & y) ^ (~x & z);
    }

    static constexpr std::uint32_t majority(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static constexpr std::uint32_t bigSigma0(std::uint32_t value) {
        return rotateRight(value, 2u) ^ rotateRight(value, 13u) ^ rotateRight(value, 22u);
    }

    static constexpr std::uint32_t bigSigma1(std::uint32_t value) {
        return rotateRight(value, 6u) ^ rotateRight(value, 11u) ^ rotateRight(value, 25u);
    }

    static constexpr std::uint32_t smallSigma0(std::uint32_t value) {
        return rotateRight(value, 7u) ^ rotateRight(value, 18u) ^ (value >> 3u);
    }

    static constexpr std::uint32_t smallSigma1(std::uint32_t value) {
        return rotateRight(value, 17u) ^ rotateRight(value, 19u) ^ (value >> 10u);
    }

    static constexpr std::array<std::uint32_t, 64> kRoundConstants = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
    };
};

} // namespace urpg::security
