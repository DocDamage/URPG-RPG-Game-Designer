#pragma once

#include <cstdint>
#include <vector>

namespace urpg::testing {

struct SnapshotPixel {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    bool operator==(const SnapshotPixel& other) const = default;
};

struct SceneSnapshot {
    int width = 0;
    int height = 0;
    std::vector<SnapshotPixel> pixels;
};

struct SnapshotComparisonResult {
    bool matches = false;
    float errorPercentage = 0.0f;
};

class SnapshotValidator {
public:
    static SnapshotComparisonResult compare(const SceneSnapshot& lhs,
                                            const SceneSnapshot& rhs,
                                            float thresholdPercentage = 0.0f) {
        if (lhs.width != rhs.width || lhs.height != rhs.height || lhs.pixels.size() != rhs.pixels.size()) {
            return {.matches = false, .errorPercentage = 100.0f};
        }

        if (lhs.pixels.empty()) {
            return {.matches = true, .errorPercentage = 0.0f};
        }

        size_t mismatch_count = 0;
        for (size_t i = 0; i < lhs.pixels.size(); ++i) {
            if (!(lhs.pixels[i] == rhs.pixels[i])) {
                ++mismatch_count;
            }
        }

        const float error_percentage =
            (static_cast<float>(mismatch_count) * 100.0f) / static_cast<float>(lhs.pixels.size());
        return {.matches = error_percentage <= thresholdPercentage, .errorPercentage = error_percentage};
    }
};

} // namespace urpg::testing
