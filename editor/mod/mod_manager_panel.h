#pragma once

#include "engine/core/mod/mod_registry.h"

#include <nlohmann/json.hpp>

#include <string>

namespace urpg::editor {

class ModManagerPanel {
public:
    ModManagerPanel() = default;

    void bindRegistry(urpg::mod::ModRegistry* registry);
    void render();

    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::mod::ModRegistry* registry_ = nullptr;
    nlohmann::json last_render_snapshot_;
};

} // namespace urpg::editor
