#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace urpg::message {

struct VisualNovelBacklogEntry {
    std::string page_id;
    std::string speaker;
    std::string text;
    std::string voice_cue;
    bool read = false;
};

struct VisualNovelPacingControls {
    bool auto_advance = false;
    bool skip_read_text = false;
    float text_speed_cps = 32.0f;
    float auto_advance_delay_seconds = 1.0f;
    std::size_t backlog_limit = 32;
};

struct VisualNovelPacingDiagnostic {
    std::string code;
    std::string target;
    std::string message;
};

struct VisualNovelPacingDocument {
    std::string id;
    VisualNovelPacingControls controls;
    std::vector<VisualNovelBacklogEntry> backlog;

    [[nodiscard]] std::vector<VisualNovelPacingDiagnostic> validate() const;
    [[nodiscard]] nlohmann::json toJson() const;

    static VisualNovelPacingDocument fromJson(const nlohmann::json& json);
};

struct VisualNovelPacingPreview {
    bool auto_advance_enabled = false;
    bool skip_available = false;
    float estimated_line_seconds = 0.0f;
    float next_advance_seconds = 0.0f;
    std::size_t backlog_entry_count = 0;
    std::size_t visible_backlog_count = 0;
    std::size_t unread_entry_count = 0;
    std::vector<VisualNovelBacklogEntry> visible_backlog;
    std::vector<std::string> runtime_commands;
    std::vector<VisualNovelPacingDiagnostic> diagnostics;
    std::string next_action;
};

[[nodiscard]] VisualNovelPacingPreview previewVisualNovelPacing(const VisualNovelPacingDocument& document);
[[nodiscard]] nlohmann::json visualNovelPacingPreviewToJson(const VisualNovelPacingPreview& preview);

} // namespace urpg::message
