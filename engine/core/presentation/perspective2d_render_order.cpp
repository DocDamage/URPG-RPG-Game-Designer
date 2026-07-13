#include "engine/core/presentation/perspective2d_render_order.h"

#include <algorithm>

namespace urpg::presentation {

void sortPerspective2DRenderOrder(std::vector<Perspective2DRenderEntry>& entries) {
    std::stable_sort(entries.begin(), entries.end(), [](const auto& left, const auto& right) {
        if (left.layer != right.layer) {
            return static_cast<int>(left.layer) < static_cast<int>(right.layer);
        }
        if (left.anchor.feetY() != right.anchor.feetY()) {
            return left.anchor.feetY() < right.anchor.feetY();
        }
        return left.objectId < right.objectId;
    });
}

} // namespace urpg::presentation
