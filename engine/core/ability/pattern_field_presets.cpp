#include "engine/core/ability/pattern_field_presets.h"

namespace urpg {

std::shared_ptr<PatternField> PatternFieldPresets::Point() {
    auto p = std::make_shared<PatternField>("Point");
    p->addPoint(0, 0);
    return p;
}

std::shared_ptr<PatternField> PatternFieldPresets::Square3x3() {
    auto p = std::make_shared<PatternField>("Square 3x3");
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            p->addPoint(x, y);
        }
    }
    return p;
}

std::shared_ptr<PatternField> PatternFieldPresets::Square5x5() {
    auto p = std::make_shared<PatternField>("Square 5x5");
    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            p->addPoint(x, y);
        }
    }
    return p;
}

std::shared_ptr<PatternField> PatternFieldPresets::Cross() {
    auto p = std::make_shared<PatternField>("Cross");
    p->addPoint(0, 0);
    p->addPoint(0, 1);
    p->addPoint(0, -1);
    p->addPoint(1, 0);
    p->addPoint(-1, 0);
    return p;
}

std::shared_ptr<PatternField> PatternFieldPresets::Diamond2() {
    auto p = std::make_shared<PatternField>("Diamond (Dist 2)");
    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            if (std::abs(x) + std::abs(y) <= 2) {
                p->addPoint(x, y);
            }
        }
    }
    return p;
}

std::shared_ptr<PatternField> PatternFieldPresets::Line3North() {
    auto p = std::make_shared<PatternField>("Line 3 North");
    p->addPoint(0, 0);
    p->addPoint(0, -1);
    p->addPoint(0, -2);
    return p;
}

} // namespace urpg
