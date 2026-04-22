#pragma once

#include <optional>
#include <string>

namespace urpg::save::lzstring {

/**
 * @brief Decompress a Base64-encoded LZString compressed string.
 *
 * This implements the LZString decompressFromBase64 algorithm used by
 * RPG Maker MV/MZ for save file compression.
 *
 * @param input Base64-encoded compressed string.
 * @return Decompressed string, or std::nullopt if input is null/empty or corrupt.
 */
std::optional<std::string> decompressFromBase64(const std::string& input);

} // namespace urpg::save::lzstring
