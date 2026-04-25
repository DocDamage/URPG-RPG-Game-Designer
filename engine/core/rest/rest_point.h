#pragma once

#include <cstdint>
#include <string>

namespace urpg::rest {

struct RestPreview {
    bool affordable = false;
    int32_t gold_after = 0;
    int32_t hp_after = 0;
    int32_t mp_after = 0;
};

struct RestPoint {
    std::string id;
    int32_t cost = 0;
    int32_t hp_restore = 0;
    int32_t mp_restore = 0;

    RestPreview preview(int32_t gold, int32_t hp, int32_t mp) const;
};

} // namespace urpg::rest
