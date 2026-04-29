#pragma once

#include <algorithm>
#include <cstdint>
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

  private:
    int32_t max_pictures_ = 100;
    std::vector<PictureTaskBinding> bindings_;
};

} // namespace urpg::message
