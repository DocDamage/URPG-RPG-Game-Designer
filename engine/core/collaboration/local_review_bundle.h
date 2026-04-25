#pragma once

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::collaboration {

struct ReviewComment {
    std::string path;
    std::string body;
    int line = 0;
};

struct LocalReviewInput {
    std::filesystem::path workspaceRoot;
    std::vector<std::filesystem::path> changedFiles;
    std::vector<ReviewComment> comments;
    std::vector<std::string> checklist;
    bool gitAvailable = true;
};

struct LocalReviewBundle {
    nlohmann::json summary = nlohmann::json::object();
    nlohmann::json handoff = nlohmann::json::object();
};

class LocalReviewBundleBuilder {
public:
    LocalReviewBundle build(const LocalReviewInput& input) const;

private:
    std::vector<std::filesystem::path> fallbackFileManifest(const std::filesystem::path& root) const;
};

} // namespace urpg::collaboration
