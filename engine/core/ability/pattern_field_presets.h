#pragma once

#include "engine/core/ability/pattern_field.h"
#include <memory>
#include <optional>
#include <vector>

namespace urpg {

/**
 * @brief Factory for common pattern presets used in Wave 2/3.
 */
class PatternFieldPresets {
public:
    enum class Usage {
        Skill,
        Item,
        Placement,
        InteractionMask
    };

    struct PresetDescriptor {
        std::string id;
        std::string display_name;
        Usage usage = Usage::Skill;
        PatternField pattern;
    };

    /**
     * @brief A single point at (0,0).
     */
    static std::shared_ptr<PatternField> Point();

    /**
     * @brief A 3x3 square centered at (0,0).
     */
    static std::shared_ptr<PatternField> Square3x3();

    /**
     * @brief A 5x5 square centered at (0,0).
     */
    static std::shared_ptr<PatternField> Square5x5();

    /**
     * @brief A cross shape (North, South, East, West) plus center.
     */
    static std::shared_ptr<PatternField> Cross();

    /**
     * @brief A diamond shape (distance <= 2).
     */
    static std::shared_ptr<PatternField> Diamond2();

    /**
     * @brief A line of 3 points starting from center and going North (0, -1), (0, -2).
     */
    static std::shared_ptr<PatternField> Line3North();

    static std::vector<PresetDescriptor> Catalog();
    static std::optional<PresetDescriptor> FindById(const std::string& preset_id);
};

} // namespace urpg
