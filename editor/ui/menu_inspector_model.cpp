#include "editor/ui/menu_inspector_model.h"

#include <algorithm>
#include <unordered_map>
#include <utility>

namespace urpg::editor {

namespace {

std::string RouteBindingLabel(urpg::MenuRouteTarget route, const std::string& custom_route_id) {
    switch (route) {
    case urpg::MenuRouteTarget::None:
        return "none";
    case urpg::MenuRouteTarget::Item:
        return "item";
    case urpg::MenuRouteTarget::Skill:
        return "skill";
    case urpg::MenuRouteTarget::Equip:
        return "equip";
    case urpg::MenuRouteTarget::Status:
        return "status";
    case urpg::MenuRouteTarget::Formation:
        return "formation";
    case urpg::MenuRouteTarget::Options:
        return "options";
    case urpg::MenuRouteTarget::Save:
        return "save";
    case urpg::MenuRouteTarget::Load:
        return "load";
    case urpg::MenuRouteTarget::GameEnd:
        return "game_end";
    case urpg::MenuRouteTarget::Codex:
        return "codex";
    case urpg::MenuRouteTarget::QuestLog:
        return "quest_log";
    case urpg::MenuRouteTarget::Encyclopedia:
        return "encyclopedia";
    case urpg::MenuRouteTarget::Custom:
        return custom_route_id.empty() ? std::string("custom:<missing>") : std::string("custom:") + custom_route_id;
    }
    return "none";
}

std::string PaneFallbackLabel(const urpg::ui::MenuPane& pane, size_t pane_index) {
    if (!pane.id.empty()) {
        return pane.id;
    }
    return "pane_" + std::to_string(pane_index);
}

std::string PaneDisplayLabel(const urpg::ui::MenuPane& pane, size_t pane_index) {
    if (!pane.displayName.empty()) {
        return pane.displayName;
    }
    return PaneFallbackLabel(pane, pane_index);
}

std::string BuildRowSummary(const MenuInspectorRow& row) {
    std::string summary = row.pane_label + " / " + row.command_label + " / " + row.route_label;
    if (row.fallback_route != urpg::MenuRouteTarget::None) {
        summary += " -> " + row.fallback_route_label;
    }
    summary += row.row_navigable ? " / navigable" : (row.command_visible ? " / blocked" : " / hidden");
    if (!row.command_registered) {
        summary += " / unregistered";
    }
    return summary;
}

} // namespace

void MenuInspectorModel::LoadFromRuntime(
    const urpg::ui::MenuSceneGraph& scene_graph,
    const urpg::ui::MenuCommandRegistry& registry,
    const urpg::ui::MenuCommandRegistry::SwitchState& switches,
    const urpg::ui::MenuCommandRegistry::VariableState& variables) {
    all_rows_.clear();
    visible_rows_.clear();
    issues_.clear();
    selected_row_index_.reset();
    summary_ = {};

    summary_.stack_depth = scene_graph.stackSize();
    const auto active_scene = scene_graph.getActiveScene();
    if (!active_scene) {
        if (summary_.stack_depth > 0) {
            MenuInspectorIssue issue;
            issue.severity = MenuInspectorIssueSeverity::Error;
            issue.code = "dangling_scene_stack";
            issue.message = "Menu scene stack references a missing active scene.";
            issues_.push_back(std::move(issue));
        }
        summary_.issue_count = issues_.size();
        RebuildVisibleRows();
        return;
    }

    summary_.active_scene_id = active_scene->getId();

    const auto& panes = active_scene->getPanes();
    summary_.total_panes = panes.size();

    std::vector<size_t> issue_count_by_row;
    std::unordered_map<std::string, std::vector<size_t>> row_indexes_by_command_id;

    const auto addIssue = [&](std::optional<size_t> row_index,
                              std::optional<size_t> pane_index,
                              std::optional<size_t> command_index,
                              MenuInspectorIssueSeverity severity,
                              std::string code,
                              std::string scene_id,
                              std::string pane_id,
                              std::string command_id,
                              std::string message) {
                    const std::string issue_code = code;
        MenuInspectorIssue issue;
        issue.severity = severity;
        issue.code = std::move(code);
        issue.pane_index = pane_index;
        issue.command_index = command_index;
        issue.scene_id = std::move(scene_id);
        issue.pane_id = std::move(pane_id);
        issue.command_id = std::move(command_id);
        issue.message = std::move(message);
        issues_.push_back(std::move(issue));

        if (row_index.has_value() && row_index.value() < issue_count_by_row.size()) {
            ++issue_count_by_row[row_index.value()];
        }

        if (issue_code == "missing_registry_entry") {
            ++summary_.missing_registry_entries;
        } else if (issue_code == "missing_primary_route_binding" ||
                   issue_code == "missing_fallback_route_binding" ||
                   issue_code == "missing_route_binding") {
            ++summary_.route_binding_issues;
        } else if (issue_code == "missing_switch_state" || issue_code == "missing_variable_state" ||
                   issue_code == "empty_rule") {
            ++summary_.rule_validation_issues;
        }
    };

    for (size_t pane_index = 0; pane_index < panes.size(); ++pane_index) {
        const auto& pane = panes[pane_index];
        const std::string pane_id = PaneFallbackLabel(pane, pane_index);
        const std::string pane_label = PaneDisplayLabel(pane, pane_index);

        if (pane.isVisible) {
            ++summary_.visible_panes;
        }
        if (pane.isActive) {
            ++summary_.active_panes;
        }

        bool pane_navigable = false;

        for (size_t command_index = 0; command_index < pane.commands.size(); ++command_index) {
            const auto& command = pane.commands[command_index];
            MenuInspectorRow row;
            row.scene_id = summary_.active_scene_id;
            row.pane_index = pane_index;
            row.pane_id = pane_id;
            row.pane_label = pane_label;
            row.command_index = command_index;
            const size_t row_index = all_rows_.size();
            issue_count_by_row.push_back(0);

            const std::string fallback_command_id = pane_id + ".command_" + std::to_string(command_index);
            row.command_id = command.id.empty() ? fallback_command_id : command.id;
            row.command_label = command.label.empty() ? row.command_id : command.label;
            row.icon_id = command.icon_id;
            row.route = command.route;
            row.route_label = RouteBindingLabel(command.route, command.custom_route_id);
            row.custom_route_id = command.custom_route_id;
            row.fallback_route = command.fallback_route;
            row.fallback_route_label = RouteBindingLabel(command.fallback_route, command.fallback_custom_route_id);
            row.fallback_custom_route_id = command.fallback_custom_route_id;
            row.priority = command.priority;
            row.pane_visible = pane.isVisible;
            row.pane_active = pane.isActive;
            row.command_registered = registry.getCommand(command.id) != nullptr;
            row.command_visible = pane.isVisible && registry.isVisible(command, switches, variables);
            row.command_enabled = pane.isVisible && registry.isEnabled(command, switches, variables);
            row.row_navigable = row.command_visible && row.command_enabled;

            ++summary_.total_commands;
            if (row.command_visible) {
                ++summary_.visible_commands;
            }
            if (row.row_navigable) {
                ++summary_.enabled_commands;
                pane_navigable = true;
            } else if (row.command_visible) {
                ++summary_.blocked_commands;
            }

            if (command.id.empty()) {
                addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                         "missing_command_id", row.scene_id, row.pane_id, row.command_id,
                         "Command is missing a stable id; the inspector synthesized a fallback id.");
            }
            if (!row.command_registered) {
                addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                         "missing_registry_entry", row.scene_id, row.pane_id, row.command_id,
                         "Command is not registered in the runtime command registry.");
            }

            if (command.route == urpg::MenuRouteTarget::None &&
                command.fallback_route == urpg::MenuRouteTarget::None) {
                addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                         "missing_route_binding", row.scene_id, row.pane_id, row.command_id,
                         "Command has no primary or fallback route binding.");
            }
            if (command.route == urpg::MenuRouteTarget::Custom && command.custom_route_id.empty()) {
                addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                         "missing_primary_route_binding", row.scene_id, row.pane_id, row.command_id,
                         "Command uses a custom primary route but does not provide custom_route_id.");
            }
            if (command.fallback_route == urpg::MenuRouteTarget::Custom &&
                command.fallback_custom_route_id.empty()) {
                addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                         "missing_fallback_route_binding", row.scene_id, row.pane_id, row.command_id,
                         "Command uses a custom fallback route but does not provide fallback_custom_route_id.");
            }

            const auto validateRules = [&](const std::vector<urpg::MenuCommandCondition>& rules,
                                           const char* scope) {
                for (size_t rule_index = 0; rule_index < rules.size(); ++rule_index) {
                    const auto& rule = rules[rule_index];
                    if (rule.switch_id.empty() && rule.variable_id.empty()) {
                        addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                                 "empty_rule", row.scene_id, row.pane_id, row.command_id,
                                 std::string(scope) + " rule " + std::to_string(rule_index) +
                                     " has no switch or variable constraint.");
                    }
                    if (!rule.switch_id.empty() && switches.find(rule.switch_id) == switches.end()) {
                        addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                                 "missing_switch_state", row.scene_id, row.pane_id, row.command_id,
                                 std::string(scope) + " rule " + std::to_string(rule_index) +
                                     " references missing switch state: " + rule.switch_id);
                    }
                    if (!rule.variable_id.empty() && variables.find(rule.variable_id) == variables.end()) {
                        addIssue(row_index, pane_index, command_index, MenuInspectorIssueSeverity::Warning,
                                 "missing_variable_state", row.scene_id, row.pane_id, row.command_id,
                                 std::string(scope) + " rule " + std::to_string(rule_index) +
                                     " references missing variable state: " + rule.variable_id);
                    }
                }
            };

            validateRules(command.visibility_rules, "visibility");
            validateRules(command.enable_rules, "enable");

            row.summary = BuildRowSummary(row);

            row_indexes_by_command_id[row.command_id].push_back(row_index);
            all_rows_.push_back(std::move(row));
        }

        if (pane_navigable) {
            ++summary_.navigable_panes;
        }
    }

    for (const auto& [command_id, row_indexes] : row_indexes_by_command_id) {
        if (row_indexes.size() <= 1) {
            continue;
        }

        ++summary_.duplicate_command_ids;

        for (size_t row_index : row_indexes) {
            const auto& row = all_rows_[row_index];
            addIssue(row_index, row.pane_index, row.command_index, MenuInspectorIssueSeverity::Warning,
                     "duplicate_command_id", row.scene_id, row.pane_id, command_id,
                     "Command id is duplicated within the active scene and may cause unstable routing.");
        }
    }

    for (size_t row_index = 0; row_index < all_rows_.size(); ++row_index) {
        all_rows_[row_index].issue_count = issue_count_by_row[row_index];
    }

    summary_.issue_count = issues_.size();
    RebuildVisibleRows();
}

void MenuInspectorModel::SetCommandIdFilter(std::optional<std::string> command_id_filter) {
    if (command_id_filter.has_value() && command_id_filter->empty()) {
        command_id_filter.reset();
    }
    command_id_filter_ = std::move(command_id_filter);
    selected_row_index_.reset();
    RebuildVisibleRows();
}

void MenuInspectorModel::SetShowIssuesOnly(bool show_issues_only) {
    show_issues_only_ = show_issues_only;
    selected_row_index_.reset();
    RebuildVisibleRows();
}

const MenuInspectorSummary& MenuInspectorModel::Summary() const {
    return summary_;
}

const std::vector<MenuInspectorRow>& MenuInspectorModel::VisibleRows() const {
    return visible_rows_;
}

const std::vector<MenuInspectorIssue>& MenuInspectorModel::Issues() const {
    return issues_;
}

bool MenuInspectorModel::SelectRow(size_t row_index) {
    if (row_index >= visible_rows_.size()) {
        selected_row_index_.reset();
        return false;
    }

    selected_row_index_ = row_index;
    return true;
}

std::optional<std::string> MenuInspectorModel::SelectedCommandId() const {
    if (!selected_row_index_.has_value()) {
        return std::nullopt;
    }

    return visible_rows_[selected_row_index_.value()].command_id;
}

void MenuInspectorModel::RebuildVisibleRows() {
    visible_rows_.clear();
    visible_rows_.reserve(all_rows_.size());

    for (const auto& row : all_rows_) {
        if (command_id_filter_.has_value() &&
            row.command_id.find(command_id_filter_.value()) == std::string::npos) {
            continue;
        }
        if (show_issues_only_) {
            if (row.issue_count == 0) {
                continue;
            }
        } else if (command_id_filter_.has_value()) {
            // Search results should include matching rows even when the command is hidden.
        } else if (!row.command_visible) {
            continue;
        }
        visible_rows_.push_back(row);
    }
}

} // namespace urpg::editor