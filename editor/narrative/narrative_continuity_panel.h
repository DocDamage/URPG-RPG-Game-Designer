#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::editor {

class NarrativeContinuityPanel {
public:
    void setDiagnostics(std::vector<std::string> diagnostics);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    std::vector<std::string> diagnostics_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
