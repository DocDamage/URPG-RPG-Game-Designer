#include "engine/core/rest/rest_point.h"

namespace urpg::rest {

RestPreview RestPoint::preview(int32_t gold, int32_t hp, int32_t mp) const {
    const bool affordable = gold >= cost;
    return RestPreview{affordable, affordable ? gold - cost : gold, affordable ? hp_restore : hp, affordable ? mp_restore : mp};
}

} // namespace urpg::rest
