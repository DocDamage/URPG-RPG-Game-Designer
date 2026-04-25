#include "engine/core/narrative/narrative_continuity_checker.h"

#include <functional>
#include <map>
#include <set>

namespace urpg::narrative {

std::vector<std::string> NarrativeContinuityChecker::check(
    const urpg::dialogue::DialogueGraph& graph,
    const std::set<std::string>& known_condition_keys) const {
    std::vector<std::string> diagnostics;
    std::set<std::string> reachable;

    std::function<void(const std::string&)> visit = [&](const std::string& node_id) {
        if (node_id.empty() || reachable.contains(node_id)) {
            return;
        }
        const auto* node = graph.findNode(node_id);
        if (!node) {
            diagnostics.push_back("unresolved_choice_target:" + node_id);
            return;
        }
        reachable.insert(node_id);
        for (const auto& choice : node->choices) {
            visit(choice.target_node_id);
        }
    };
    visit(graph.startNode());

    bool has_ending = false;
    for (const auto& [id, node] : graph.nodes()) {
        if (!reachable.contains(id)) {
            diagnostics.push_back("orphaned_node:" + id);
        }
        if (node.localization_key.empty()) {
            diagnostics.push_back("missing_localization_key:" + id);
        }
        has_ending = has_ending || node.ending;

        for (const auto& choice : node.choices) {
            if (!choice.target_node_id.empty() && !graph.findNode(choice.target_node_id)) {
                diagnostics.push_back("unresolved_choice:" + choice.id);
            }
            std::map<std::string, std::pair<int, int>> ranges;
            for (const auto& condition : choice.conditions) {
                if (!known_condition_keys.contains(condition.key)) {
                    diagnostics.push_back("unknown_condition_key:" + condition.key);
                }
                auto& range = ranges[condition.key];
                if (range.first == 0 && range.second == 0) {
                    range = {-1000000, 1000000};
                }
                if (condition.op == ">=") {
                    range.first = std::max(range.first, condition.value);
                } else if (condition.op == "<=") {
                    range.second = std::min(range.second, condition.value);
                }
                if (range.first > range.second) {
                    diagnostics.push_back("impossible_condition:" + choice.id + ":" + condition.key);
                }
            }
        }
    }
    if (!has_ending) {
        diagnostics.push_back("missing_ending");
    }
    return diagnostics;
}

} // namespace urpg::narrative
