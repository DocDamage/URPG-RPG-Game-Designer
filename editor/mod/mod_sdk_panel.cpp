#include "editor/mod/mod_sdk_panel.h"

namespace urpg::editor {

urpg::mod::ModSdkSampleValidationResult ModSdkPanel::validateSample(const std::filesystem::path& sampleRoot) {
    urpg::mod::ModSdkSampleValidator validator;
    last_result_ = validator.validateSample(sampleRoot);
    return last_result_;
}

void ModSdkPanel::render() {
    last_render_snapshot_ = {
        {"passed", last_result_.passed},
        {"issue_count", last_result_.issues.size()},
        {"sample_id", last_result_.manifest.value("id", "")},
    };
}

nlohmann::json ModSdkPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
