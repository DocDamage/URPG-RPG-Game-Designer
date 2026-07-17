#include "editor/ui/editor_command_palette.h"

#include <algorithm>
#include <cctype>
#include <numeric>
#include <set>
#include <utility>

namespace urpg::editor {
namespace {

std::string normalized(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

int matchScore(const EditorCommandDescriptor& command, const std::string& query) {
    if (query.empty()) return 1;
    const auto id = normalized(command.id);
    const auto label = normalized(command.label);
    const auto category = normalized(command.category);
    if (id == query || label == query) return 100;
    if (label.starts_with(query)) return 80;
    if (id.starts_with(query)) return 70;
    if (label.find(query) != std::string::npos) return 60;
    if (id.find(query) != std::string::npos) return 50;
    if (category.find(query) != std::string::npos) return 40;
    for (const auto& keyword : command.keywords) {
        if (normalized(keyword).find(query) != std::string::npos) return 30;
    }
    if (normalized(command.help).find(query) != std::string::npos) return 20;
    if (normalized(command.requirement).find(query) != std::string::npos) return 10;
    return 0;
}

std::string searchableText(const EditorCommandDescriptor& command) {
    std::string text = command.id + "\n" + command.label + "\n" + command.category + "\n" + command.help +
                       "\n" + command.requirement;
    for (const auto& keyword : command.keywords) text += "\n" + keyword;
    return normalized(text);
}

std::set<std::string> searchGrams(const std::string& text) {
    std::set<std::string> grams;
    for (std::size_t length = 1; length <= 3; ++length) {
        if (text.size() < length) break;
        for (std::size_t offset = 0; offset + length <= text.size(); ++offset) {
            grams.insert(text.substr(offset, length));
        }
    }
    return grams;
}

void rankSearchResults(std::vector<EditorCommandSearchResult>& results, const std::size_t limit) {
    std::sort(results.begin(), results.end(), [](const auto& left, const auto& right) {
        if (left.score != right.score) return left.score > right.score;
        if (left.command.label != right.command.label) return left.command.label < right.command.label;
        return left.command.id < right.command.id;
    });
    if (results.size() > limit) results.resize(limit);
}

} // namespace

EditorCommandSearchJob::EditorCommandSearchJob(const EditorCommandPalette* palette, std::string query,
                                               const std::size_t limit,
                                               std::vector<std::size_t> candidate_indexes,
                                               std::vector<std::string> recent_ids)
    : palette_(palette), query_(std::move(query)), limit_(limit), candidate_indexes_(std::move(candidate_indexes)),
      recent_ids_(std::move(recent_ids)), complete_(palette == nullptr || limit == 0 || candidate_indexes_.empty()) {}

bool EditorCommandSearchJob::advance(const std::size_t maximum_candidates) {
    if (complete_ || maximum_candidates == 0) return complete_;
    const auto end = cursor_ + std::min(maximum_candidates, candidate_indexes_.size() - cursor_);
    for (; cursor_ < end; ++cursor_) {
        const auto& command = palette_->commands_[candidate_indexes_[cursor_]];
        const int score = matchScore(command, query_);
        if (score == 0) continue;
        const bool recent = std::find(recent_ids_.begin(), recent_ids_.end(), command.id) != recent_ids_.end();
        results_.push_back({command, score + (recent ? 5 : 0), recent});
    }
    rankSearchResults(results_, limit_);
    complete_ = cursor_ == candidate_indexes_.size();
    return complete_;
}

bool EditorCommandPalette::registerCommand(EditorCommandDescriptor command) {
    if (command.id.empty() || command.label.empty() || command.category.empty() || command.help.empty() ||
        command_index_by_id_.contains(command.id)) {
        return false;
    }
    const auto commandIndex = commands_.size();
    commands_.push_back(std::move(command));
    command_index_by_id_.emplace(commands_.back().id, commandIndex);
    for (const auto& gram : searchGrams(searchableText(commands_.back()))) {
        search_postings_[gram].push_back(commandIndex);
    }
    ++search_index_revision_;
    return true;
}

std::vector<std::size_t> EditorCommandPalette::searchCandidates(const std::string& normalized_query) const {
    std::vector<std::size_t> candidates;
    if (normalized_query.empty()) {
        candidates.resize(commands_.size());
        std::iota(candidates.begin(), candidates.end(), 0);
    } else {
        const auto gramLength = std::min<std::size_t>(3, normalized_query.size());
        bool first = true;
        for (std::size_t offset = 0; offset + gramLength <= normalized_query.size(); ++offset) {
            const auto posting = search_postings_.find(normalized_query.substr(offset, gramLength));
            if (posting == search_postings_.end()) return {};
            if (first) {
                candidates = posting->second;
                first = false;
                continue;
            }
            std::vector<std::size_t> intersection;
            std::set_intersection(candidates.begin(), candidates.end(), posting->second.begin(), posting->second.end(),
                                  std::back_inserter(intersection));
            candidates = std::move(intersection);
            if (candidates.empty()) return {};
        }
    }
    return candidates;
}

EditorCommandSearchJob EditorCommandPalette::beginSearch(const std::string_view query, const std::size_t limit) const {
    const auto needle = normalized(query);
    auto candidates = limit == 0 ? std::vector<std::size_t>{} : searchCandidates(needle);
    last_search_candidate_count_ = candidates.size();
    return EditorCommandSearchJob(this, needle, limit, std::move(candidates), recent_ids_);
}

std::vector<EditorCommandSearchResult> EditorCommandPalette::search(const std::string_view query,
                                                                     const std::size_t limit) const {
    auto job = beginSearch(query, limit);
    while (!job.complete()) (void)job.advance();
    return job.results();
}

bool EditorCommandPalette::recordAction(const std::string_view command_id) {
    const auto command = command_index_by_id_.find(std::string(command_id));
    if (command == command_index_by_id_.end() || !commands_[command->second].enabled) return false;
    recent_ids_.erase(std::remove(recent_ids_.begin(), recent_ids_.end(), command_id), recent_ids_.end());
    recent_ids_.insert(recent_ids_.begin(), std::string(command_id));
    if (recent_ids_.size() > 20) recent_ids_.resize(20);
    return true;
}

std::vector<EditorCommandDescriptor> EditorCommandPalette::recentActions(const std::size_t limit) const {
    std::vector<EditorCommandDescriptor> result;
    for (const auto& id : recent_ids_) {
        const auto command = command_index_by_id_.find(id);
        if (command != command_index_by_id_.end()) result.push_back(commands_[command->second]);
        if (result.size() == limit) break;
    }
    return result;
}

std::vector<EditorCommandDescriptor> EditorCommandPalette::shortcutDiscovery() const {
    std::vector<EditorCommandDescriptor> result;
    std::copy_if(commands_.begin(), commands_.end(), std::back_inserter(result),
                 [](const auto& command) { return !command.shortcut.empty(); });
    std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
        return left.shortcut != right.shortcut ? left.shortcut < right.shortcut : left.id < right.id;
    });
    return result;
}

EditorCommandPalette buildGoldenLoopCommandPalette() {
    EditorCommandPalette palette;
    (void)palette.registerCommand({"project.create", "Open New Project Wizard", "Project", {"new", "template"}, "Ctrl+Shift+N",
                                   "Startup shell is open.", "Open the governed project wizard and choose a certified template."});
    (void)palette.registerCommand({"map.open", "Open Map", "Map", {"level", "canvas"}, "Ctrl+Alt+M",
                                   "A project is open.", "Open the unified Map authoring workspace."});
    (void)palette.registerCommand({"event.create", "Open Event Authoring", "Event", {"dialogue", "interaction"}, "Ctrl+Shift+E",
                                   "A writable map is open.", "Open the active Map's native event-authoring route."});
    (void)palette.registerCommand({"project.save", "Save Project", "Project", {"write", "persist"}, "Ctrl+S",
                                   "A project is open.", "Save changed project documents through their owners."});
    (void)palette.registerCommand({"playtest.current_map", "Playtest Current Map", "Playtest", {"run", "preview"}, "F6",
                                   "The active map has no blocking diagnostics.", "Launch a private current-map playtest overlay."});
    (void)palette.registerCommand({"project.health", "Open Project Health", "Diagnostics", {"validate", "fix"}, "Ctrl+Shift+H",
                                   "A project is open.", "Review blockers, warnings, and suggested fixes."});
    (void)palette.registerCommand({"export.validate", "Open Export Validation", "Export", {"package", "release"}, "Ctrl+Shift+B",
                                   "A project is open.", "Open governed export diagnostics without starting blocking I/O."});
    return palette;
}

} // namespace urpg::editor
