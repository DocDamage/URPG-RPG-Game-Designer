#pragma once

#include "editor/database/database_table_model.h"
#include "engine/core/database/rpg_database.h"

namespace urpg::editor {

struct DatabasePanelSnapshot {
    std::size_t actor_count = 0;
    std::size_t item_count = 0;
    std::size_t diagnostic_count = 0;
    std::size_t table_count = 0;
};

class DatabasePanel {
public:
    void setDatabase(database::RpgDatabase database);
    DatabasePanelSnapshot snapshot() const;
    void render();
    ProjectDatabaseTableModel& tables() { return tables_; }
    const ProjectDatabaseTableModel& tables() const { return tables_; }
    const DatabasePanelSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    database::RpgDatabase database_;
    ProjectDatabaseTableModel tables_;
    DatabasePanelSnapshot last_render_snapshot_{};
};

} // namespace urpg::editor
