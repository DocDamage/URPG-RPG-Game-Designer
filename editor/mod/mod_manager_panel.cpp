#include "editor/mod/mod_manager_panel.h"

namespace urpg::editor {

void ModManagerPanel::bindRegistry(urpg::mod::ModRegistry* registry) {
    registry_ = registry;
}

void ModManagerPanel::render() {
    nlohmann::json snapshot;
    snapshot["registered_count"] = 0;
    snapshot["active_count"] = 0;
    snapshot["resolved_load_order"] = nlohmann::json::array();
    snapshot["cycle_warning"] = nullptr;

    if (!registry_) {
        last_render_snapshot_ = snapshot;
        return;
    }

    const auto activeMods = registry_->listActiveMods();
    snapshot["active_count"] = activeMods.size();

    try {
        const auto loadOrder = registry_->resolveLoadOrder();
        snapshot["resolved_load_order"] = loadOrder;
        snapshot["registered_count"] = loadOrder.size();
    } catch (const std::runtime_error& e) {
        snapshot["cycle_warning"] = e.what();
        snapshot["registered_count"] = activeMods.size();
    }

    last_render_snapshot_ = snapshot;
}

nlohmann::json ModManagerPanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
