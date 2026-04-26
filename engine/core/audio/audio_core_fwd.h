#pragma once

#include <cstdint>

namespace urpg::audio {

using AudioHandle = std::uint64_t;

enum class AudioCategory : std::uint8_t { BGM, BGS, SE, ME, System };

} // namespace urpg::audio
