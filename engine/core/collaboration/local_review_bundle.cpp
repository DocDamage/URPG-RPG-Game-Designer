#include "engine/core/collaboration/local_review_bundle.h"

#include <algorithm>

namespace {

std::string rel(const std::filesystem::path& path, const std::filesystem::path& root) {
    std::error_code ec;
    const auto relative = std::filesystem::relative(path, root, ec);
    return (ec ? path : relative).generic_string();
}

} // namespace

namespace urpg::collaboration {

LocalReviewBundle LocalReviewBundleBuilder::build(const LocalReviewInput& input) const {
    LocalReviewBundle bundle;
    const auto files = input.gitAvailable ? input.changedFiles : fallbackFileManifest(input.workspaceRoot);

    bundle.summary["schema"] = "urpg.local_review.v1";
    bundle.summary["source"] = input.gitAvailable ? "git" : "file_manifest";
    bundle.summary["changed_files"] = nlohmann::json::array();
    for (const auto& file : files) {
        bundle.summary["changed_files"].push_back(rel(file, input.workspaceRoot));
    }

    bundle.summary["comments"] = nlohmann::json::array();
    for (const auto& comment : input.comments) {
        bundle.summary["comments"].push_back({
            {"path", comment.path},
            {"line", comment.line},
            {"body", comment.body},
        });
    }

    bundle.summary["checklist"] = input.checklist;
    bundle.handoff["schema"] = "urpg.local_review_handoff.v1";
    bundle.handoff["summary"] = bundle.summary;
    bundle.handoff["ready_for_async_review"] = true;
    return bundle;
}

std::vector<std::filesystem::path> LocalReviewBundleBuilder::fallbackFileManifest(const std::filesystem::path& root) const {
    std::vector<std::filesystem::path> files;
    if (root.empty() || !std::filesystem::exists(root)) {
        return files;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (entry.is_regular_file()) {
            const auto path = entry.path();
            const auto text = path.generic_string();
            if (text.find("/.git/") == std::string::npos) {
                files.push_back(path);
            }
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

} // namespace urpg::collaboration
