#pragma once

#include <algorithm>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::message {

struct PictureTaskBinding {
    int32_t picture_id = 0;
    std::string task_id;
    std::string common_event_id;
    std::string trigger = "click";
    bool enabled = true;
};

struct PictureTaskDiagnostic {
    std::string code;
    std::string message;
    int32_t picture_id = 0;
};

struct PictureRuntimeSlot {
    int32_t picture_id = 0;
    std::string asset_id;
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t z = 0;
    float opacity = 1.0F;
    bool visible = true;
};

struct PictureRuntimeRow {
    int32_t picture_id = 0;
    std::string asset_id;
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t z = 0;
    float opacity = 1.0F;
    bool visible = true;
    bool clickable = false;
    bool hoverable = false;
    bool hovered = false;
    std::vector<PictureTaskBinding> bindings;
    std::vector<std::string> common_event_ids;
};

struct PictureRuntimePreview {
    int32_t max_pictures = 100;
    int32_t pointer_x = 0;
    int32_t pointer_y = 0;
    size_t visible_picture_count = 0;
    size_t bound_picture_count = 0;
    size_t clickable_picture_count = 0;
    size_t hoverable_picture_count = 0;
    std::vector<PictureRuntimeRow> rows;
    std::vector<PictureTaskDiagnostic> diagnostics;

    [[nodiscard]] nlohmann::json toJson() const {
        nlohmann::json row_json = nlohmann::json::array();
        for (const auto& row : rows) {
            nlohmann::json bindings_json = nlohmann::json::array();
            for (const auto& binding : row.bindings) {
                bindings_json.push_back({{"picture_id", binding.picture_id},
                                         {"task_id", binding.task_id},
                                         {"common_event_id", binding.common_event_id},
                                         {"trigger", binding.trigger},
                                         {"enabled", binding.enabled}});
            }
            row_json.push_back({{"picture_id", row.picture_id},
                                {"asset_id", row.asset_id},
                                {"x", row.x},
                                {"y", row.y},
                                {"width", row.width},
                                {"height", row.height},
                                {"z", row.z},
                                {"opacity", row.opacity},
                                {"visible", row.visible},
                                {"clickable", row.clickable},
                                {"hoverable", row.hoverable},
                                {"hovered", row.hovered},
                                {"bindings", bindings_json},
                                {"common_event_ids", row.common_event_ids}});
        }

        nlohmann::json diagnostic_json = nlohmann::json::array();
        for (const auto& diagnostic : diagnostics) {
            diagnostic_json.push_back({{"code", diagnostic.code},
                                       {"message", diagnostic.message},
                                       {"picture_id", diagnostic.picture_id}});
        }
        return {{"max_pictures", max_pictures},
                {"pointer_x", pointer_x},
                {"pointer_y", pointer_y},
                {"visible_picture_count", visible_picture_count},
                {"bound_picture_count", bound_picture_count},
                {"clickable_picture_count", clickable_picture_count},
                {"hoverable_picture_count", hoverable_picture_count},
                {"rows", row_json},
                {"diagnostics", diagnostic_json}};
    }
};

class PictureTaskDocument {
  public:
    void setMaxPictures(int32_t max_pictures) { max_pictures_ = std::max(1, max_pictures); }
    [[nodiscard]] int32_t maxPictures() const { return max_pictures_; }

    void addBinding(PictureTaskBinding binding) { bindings_.push_back(std::move(binding)); }
    [[nodiscard]] const std::vector<PictureTaskBinding>& bindings() const { return bindings_; }

    [[nodiscard]] std::vector<PictureTaskBinding> bindingsForPicture(int32_t picture_id) const {
        std::vector<PictureTaskBinding> result;
        for (const auto& binding : bindings_) {
            if (binding.picture_id == picture_id && binding.enabled) {
                result.push_back(binding);
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<PictureTaskDiagnostic> validate() const {
        std::vector<PictureTaskDiagnostic> diagnostics;
        for (const auto& binding : bindings_) {
            if (binding.picture_id <= 0 || binding.picture_id > max_pictures_) {
                diagnostics.push_back({"picture_id_out_of_range", "Picture task binding references an unavailable picture slot.", binding.picture_id});
            }
            if (binding.task_id.empty()) {
                diagnostics.push_back({"missing_picture_task_id", "Picture task binding needs a task id.", binding.picture_id});
            }
            if (binding.common_event_id.empty()) {
                diagnostics.push_back({"missing_picture_common_event", "Picture task binding needs a common event id.", binding.picture_id});
            }
        }
        return diagnostics;
    }

    [[nodiscard]] PictureRuntimePreview previewRuntime(const std::vector<PictureRuntimeSlot>& pictures,
                                                       int32_t pointer_x,
                                                       int32_t pointer_y) const {
        PictureRuntimePreview preview;
        preview.max_pictures = max_pictures_;
        preview.pointer_x = pointer_x;
        preview.pointer_y = pointer_y;
        preview.diagnostics = validate();

        std::vector<PictureRuntimeSlot> sorted_pictures = pictures;
        std::sort(sorted_pictures.begin(), sorted_pictures.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs.z != rhs.z) {
                return lhs.z < rhs.z;
            }
            return lhs.picture_id < rhs.picture_id;
        });

        for (const auto& picture : sorted_pictures) {
            if (picture.picture_id <= 0 || picture.picture_id > max_pictures_) {
                preview.diagnostics.push_back({"picture_runtime_slot_out_of_range",
                                               "Runtime picture slot is outside the configured high-count range.",
                                               picture.picture_id});
                continue;
            }
            PictureRuntimeRow row;
            row.picture_id = picture.picture_id;
            row.asset_id = picture.asset_id;
            row.x = picture.x;
            row.y = picture.y;
            row.width = std::max(0, picture.width);
            row.height = std::max(0, picture.height);
            row.z = picture.z;
            row.opacity = std::clamp(picture.opacity, 0.0F, 1.0F);
            row.visible = picture.visible && row.opacity > 0.0F && row.width > 0 && row.height > 0;
            row.bindings = bindingsForPicture(picture.picture_id);
            for (const auto& binding : row.bindings) {
                if (std::find(row.common_event_ids.begin(), row.common_event_ids.end(), binding.common_event_id) ==
                    row.common_event_ids.end()) {
                    row.common_event_ids.push_back(binding.common_event_id);
                }
                if (binding.trigger == "click") {
                    row.clickable = true;
                }
                if (binding.trigger == "hover") {
                    row.hoverable = true;
                }
            }
            row.hovered = row.visible && pointer_x >= row.x && pointer_x < row.x + row.width &&
                          pointer_y >= row.y && pointer_y < row.y + row.height;
            if (row.visible) {
                ++preview.visible_picture_count;
            }
            if (!row.bindings.empty()) {
                ++preview.bound_picture_count;
            }
            if (row.clickable) {
                ++preview.clickable_picture_count;
            }
            if (row.hoverable) {
                ++preview.hoverable_picture_count;
            }
            preview.rows.push_back(std::move(row));
        }
        return preview;
    }

  private:
    int32_t max_pictures_ = 100;
    std::vector<PictureTaskBinding> bindings_;
};

} // namespace urpg::message
