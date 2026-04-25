#include "editor/database/database_panel.h"

#include <utility>

namespace urpg::editor {

void DatabasePanel::setDatabase(database::RpgDatabase database) {
    database_ = std::move(database);
}

DatabasePanelSnapshot DatabasePanel::snapshot() const {
    return DatabasePanelSnapshot{database_.actors().size(), database_.items().size(), database_.validate().size()};
}

void DatabasePanel::render() {
    last_render_snapshot_ = snapshot();
}

} // namespace urpg::editor
