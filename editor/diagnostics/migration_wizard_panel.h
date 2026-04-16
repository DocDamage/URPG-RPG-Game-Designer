#pragma once

#include "migration_wizard_model.h"
#include <memory>

namespace urpg::editor {

/**
 * @brief GUI controller for the Project Migration Wizard.
 */
class MigrationWizardPanel {
public:
    MigrationWizardPanel() : m_model(std::make_shared<MigrationWizardModel>()) {}

    void onProjectUpdateRequested(const nlohmann::json& project_data) {
        m_model->runFullMigration(project_data);
    }

    std::shared_ptr<MigrationWizardModel> getModel() const { return m_model; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    std::shared_ptr<MigrationWizardModel> m_model;
    bool m_visible = false;
};

} // namespace urpg::editor
