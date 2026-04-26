#pragma once

#include "editor/diagnostics/diagnostics_workspace.h"
#include <functional>
#include <string>

namespace urpg::editor {

/**
 * @brief A lightweight facade for emitting diagnostic snapshots from the DiagnosticsWorkspace.
 *
 * This class provides a bridge between the core DiagnosticsWorkspace and external consumers
 * (like editor shells or UI transports) that need to periodically or on-demand capture
 * the current state of diagnostic data.
 */
class DiagnosticsFacade {
  public:
    explicit DiagnosticsFacade(DiagnosticsWorkspace& workspace);

    /**
     * @brief Emits a JSON snapshot of the current diagnostics state.
     *
     * @return std::string A JSON-formatted string representing the diagnostics snapshot.
     */
    std::string emitSnapshot() const;

    /**
     * @brief Triggers a refresh of all diagnostic panels before emitting a snapshot.
     */
    void refreshAndEmit(std::function<void(std::string_view)> callback) const;

  private:
    DiagnosticsWorkspace& workspace_;
};

} // namespace urpg::editor
