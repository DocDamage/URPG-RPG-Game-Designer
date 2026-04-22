#pragma once

#include <cstddef>
#include <cstdint>

#include <nlohmann/json.hpp>

#include "engine/core/analytics/analytics_dispatcher.h"

namespace urpg::editor {

class AnalyticsPanel {
public:
    void bindDispatcher(urpg::analytics::AnalyticsDispatcher* dispatcher);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::analytics::AnalyticsDispatcher* m_dispatcher = nullptr;
    nlohmann::json m_lastSnapshot;
};

}  // namespace urpg::editor
