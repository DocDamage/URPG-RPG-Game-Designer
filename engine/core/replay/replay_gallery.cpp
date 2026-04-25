#include "engine/core/replay/replay_gallery.h"

#include <algorithm>
#include <utility>

namespace urpg::replay {

void ReplayGallery::add(ReplayArtifact artifact) {
    artifacts_.push_back(std::move(artifact));
}

std::vector<ReplayArtifact> ReplayGallery::findByLabel(const std::string& label) const {
    std::vector<ReplayArtifact> matches;
    for (const auto& artifact : artifacts_) {
        if (artifact.labels.count(label) > 0) {
            matches.push_back(artifact);
        }
    }
    std::stable_sort(matches.begin(), matches.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.id < rhs.id;
    });
    return matches;
}

} // namespace urpg::replay
