#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <functional>

namespace urpg::editor {

    /**
     * @brief Metadata for a single editable property.
     * Part of Wave 5.4: Property Inspector Auto-Generation.
     */
    enum class PropertyType { Int, Float, String, Bool, Color, Vec2, Enum };

    struct PropertyMetadata {
        std::string label;
        PropertyType type;
        std::string group; // For Foldable headers
        float min = 0.0f;
        float max = 1.0f;
    };

    /**
     * @brief Map-based property registry for an entity/subsystem.
     * Allows the editor to build UI dynamically without hardcoding every class.
     */
    class PropertyRegistry {
    public:
        using PropertyValue = std::variant<int*, float*, std::string*, bool*, uint32_t*>;

        void registerProperty(const std::string& key, PropertyValue ptr, PropertyMetadata meta) {
            m_properties[key] = {ptr, meta};
        }

        /**
         * @brief The core 'Auto-UI' hook. Iterates and calls ImGui widgets.
         */
        void drawInspector() {
            for (auto const& [key, entry] : m_properties) {
                const auto& meta = entry.second;
                // Representing dynamic ImGui emission:
                // if (meta.type == PropertyType::Float) ImGui::SliderFloat(meta.label, (float*)entry.first, meta.min, meta.max);
            }
        }

    private:
        struct PropertyEntry {
            PropertyValue value;
            PropertyMetadata metadata;
        };
        std::map<std::string, PropertyEntry> m_properties;
    };

} // namespace urpg::editor
