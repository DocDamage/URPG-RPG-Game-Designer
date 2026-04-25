#pragma once

#include "engine/core/collaboration/local_review_bundle.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class LocalReviewPanel {
public:
    urpg::collaboration::LocalReviewBundle buildBundle(const urpg::collaboration::LocalReviewInput& input);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::collaboration::LocalReviewBundle last_bundle_;
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
