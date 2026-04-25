#pragma once

#include "engine/core/replay/replay_recorder.h"

#include <string>
#include <vector>

namespace urpg::replay {

class ReplayGallery {
public:
    void add(ReplayArtifact artifact);
    std::vector<ReplayArtifact> findByLabel(const std::string& label) const;
    const std::vector<ReplayArtifact>& artifacts() const { return artifacts_; }

private:
    std::vector<ReplayArtifact> artifacts_;
};

} // namespace urpg::replay
