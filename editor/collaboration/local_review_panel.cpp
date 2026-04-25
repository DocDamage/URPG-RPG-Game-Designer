#include "editor/collaboration/local_review_panel.h"

namespace urpg::editor {

urpg::collaboration::LocalReviewBundle LocalReviewPanel::buildBundle(
    const urpg::collaboration::LocalReviewInput& input) {
    urpg::collaboration::LocalReviewBundleBuilder builder;
    last_bundle_ = builder.build(input);
    return last_bundle_;
}

void LocalReviewPanel::render() {
    last_render_snapshot_ = {
        {"source", last_bundle_.summary.value("source", "none")},
        {"changed_file_count", last_bundle_.summary.value("changed_files", nlohmann::json::array()).size()},
        {"comment_count", last_bundle_.summary.value("comments", nlohmann::json::array()).size()},
    };
}

nlohmann::json LocalReviewPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
