#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::editor {

struct EditorCommandDescriptor {
    std::string id;
    std::string label;
    std::string category;
    std::vector<std::string> keywords;
    std::string shortcut;
    std::string requirement;
    std::string help;
    bool enabled = true;
};

struct EditorCommandSearchResult {
    EditorCommandDescriptor command;
    int score = 0;
    bool recent = false;
};

class EditorCommandPalette;

class EditorCommandSearchJob {
public:
    static constexpr std::size_t kDefaultMaximumCandidatesPerSlice = 64;

    bool advance(std::size_t maximum_candidates = kDefaultMaximumCandidatesPerSlice);
    bool complete() const { return complete_; }
    std::size_t candidateCount() const { return candidate_indexes_.size(); }
    std::size_t processedCount() const { return cursor_; }
    const std::vector<EditorCommandSearchResult>& results() const { return results_; }

private:
    friend class EditorCommandPalette;
    EditorCommandSearchJob(const EditorCommandPalette* palette, std::string query, std::size_t limit,
                           std::vector<std::size_t> candidate_indexes, std::vector<std::string> recent_ids);

    const EditorCommandPalette* palette_ = nullptr;
    std::string query_;
    std::size_t limit_ = 0;
    std::vector<std::size_t> candidate_indexes_;
    std::vector<std::string> recent_ids_;
    std::size_t cursor_ = 0;
    bool complete_ = false;
    std::vector<EditorCommandSearchResult> results_;
};

class EditorCommandPalette {
public:
    bool registerCommand(EditorCommandDescriptor command);
    // The returned job borrows this palette; finish or discard it before the
    // palette is destroyed or moved.
    EditorCommandSearchJob beginSearch(std::string_view query, std::size_t limit = 20) const;
    std::vector<EditorCommandSearchResult> search(std::string_view query, std::size_t limit = 20) const;
    bool recordAction(std::string_view command_id);
    std::vector<EditorCommandDescriptor> recentActions(std::size_t limit = 10) const;
    std::vector<EditorCommandDescriptor> shortcutDiscovery() const;
    const std::vector<EditorCommandDescriptor>& commands() const { return commands_; }
    uint64_t searchIndexRevision() const { return search_index_revision_; }
    std::size_t lastSearchCandidateCount() const { return last_search_candidate_count_; }

private:
    friend class EditorCommandSearchJob;
    std::vector<std::size_t> searchCandidates(const std::string& normalized_query) const;

    std::vector<EditorCommandDescriptor> commands_;
    std::vector<std::string> recent_ids_;
    std::map<std::string, std::size_t> command_index_by_id_;
    std::map<std::string, std::vector<std::size_t>> search_postings_;
    uint64_t search_index_revision_ = 0;
    mutable std::size_t last_search_candidate_count_ = 0;
};

EditorCommandPalette buildGoldenLoopCommandPalette();

} // namespace urpg::editor
