#pragma once

#include "save_types.h"
#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace urpg {

/**
 * @brief Registry for controlling save metadata schema and validation.
 * 
 * In the URPG engine, the metadata structure for save slots is flexible.
 * The registry defines which keys are expected and how they should be handled
 * for presentation in the UI (e.g., Save/Load menus).
 */
class SaveMetadataRegistry {
public:
    struct MetadataField {
        std::string key;
        std::string display_label;
        bool required = false;
        std::string default_value;
    };

    void registerField(const MetadataField& field) {
        _fields[field.key] = field;
    }

    const std::map<std::string, MetadataField>& getFields() const {
        return _fields;
    }

    /**
     * @brief Loads fields from a JSON schema (from save_policies.json).
     */
    bool loadFromSchema(const nlohmann::json& schema) {
        if (!schema.is_object() || !schema.contains("metadata_fields")) {
            return false;
        }

        const auto& fields = schema["metadata_fields"];
        if (!fields.is_array()) {
            return false;
        }

        for (const auto& item : fields) {
            if (!item.contains("key")) continue;
            
            MetadataField field;
            field.key = item["key"].get<std::string>();
            field.display_label = item.value("display_label", field.key);
            field.required = item.value("required", false);
            field.default_value = item.value("default_value", "");
            registerField(field);
        }
        return true;
    }

    /**
     * @brief Populates default values for missing keys in custom_metadata.
     */
    void applyDefaults(SaveSlotMeta& meta) const {
        for (const auto& [key, field] : _fields) {
            if (meta.custom_metadata.find(key) == meta.custom_metadata.end()) {
                meta.custom_metadata[key] = field.default_value;
            }
        }
    }

private:
    std::map<std::string, MetadataField> _fields;
};

} // namespace urpg
