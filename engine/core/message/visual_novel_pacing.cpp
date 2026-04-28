#include "engine/core/message/visual_novel_pacing.h"

#include <algorithm>

namespace urpg::message {

namespace {

VisualNovelBacklogEntry backlogEntryFromJson(const nlohmann::json& json) {
    return {
        json.value("page_id", ""),
        json.value("speaker", ""),
        json.value("text", ""),
        json.value("voice_cue", ""),
        json.value("read", false),
    };
}

nlohmann::json backlogEntryToJson(const VisualNovelBacklogEntry& entry) {
    return {
        {"page_id", entry.page_id},
        {"speaker", entry.speaker},
        {"text", entry.text},
        {"voice_cue", entry.voice_cue},
        {"read", entry.read},
    };
}

} // namespace

std::vector<VisualNovelPacingDiagnostic> VisualNovelPacingDocument::validate() const {
    std::vector<VisualNovelPacingDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_id", "document", "Visual novel pacing document requires an id."});
    }
    if (controls.text_speed_cps <= 0.0f) {
        diagnostics.push_back({"invalid_text_speed", "controls", "Text speed must be greater than zero."});
    }
    if (controls.auto_advance_delay_seconds < 0.0f) {
        diagnostics.push_back({"invalid_auto_delay", "controls", "Auto-advance delay cannot be negative."});
    }
    if (controls.backlog_limit == 0) {
        diagnostics.push_back({"invalid_backlog_limit", "controls", "Backlog limit must keep at least one entry."});
    }

    for (const auto& entry : backlog) {
        const auto target = entry.page_id.empty() ? "backlog" : entry.page_id;
        if (entry.page_id.empty()) {
            diagnostics.push_back({"missing_page_id", target, "Backlog entry requires a page id."});
        }
        if (entry.speaker.empty()) {
            diagnostics.push_back({"missing_speaker", target, "Backlog entry requires a speaker."});
        }
        if (entry.text.empty()) {
            diagnostics.push_back({"missing_text", target, "Backlog entry requires text."});
        }
    }
    return diagnostics;
}

nlohmann::json VisualNovelPacingDocument::toJson() const {
    nlohmann::json backlogJson = nlohmann::json::array();
    for (const auto& entry : backlog) {
        backlogJson.push_back(backlogEntryToJson(entry));
    }
    return {
        {"schema", "urpg.visual_novel_pacing.v1"},
        {"id", id},
        {"controls", {
            {"auto_advance", controls.auto_advance},
            {"skip_read_text", controls.skip_read_text},
            {"text_speed_cps", controls.text_speed_cps},
            {"auto_advance_delay_seconds", controls.auto_advance_delay_seconds},
            {"backlog_limit", controls.backlog_limit},
        }},
        {"backlog", backlogJson},
    };
}

VisualNovelPacingDocument VisualNovelPacingDocument::fromJson(const nlohmann::json& json) {
    VisualNovelPacingDocument document;
    document.id = json.value("id", "");
    const auto controls = json.value("controls", nlohmann::json::object());
    document.controls.auto_advance = controls.value("auto_advance", false);
    document.controls.skip_read_text = controls.value("skip_read_text", false);
    document.controls.text_speed_cps = controls.value("text_speed_cps", 32.0f);
    document.controls.auto_advance_delay_seconds = controls.value("auto_advance_delay_seconds", 1.0f);
    document.controls.backlog_limit = controls.value("backlog_limit", static_cast<std::size_t>(32));
    for (const auto& entry : json.value("backlog", nlohmann::json::array())) {
        document.backlog.push_back(backlogEntryFromJson(entry));
    }
    return document;
}

VisualNovelPacingPreview previewVisualNovelPacing(const VisualNovelPacingDocument& document) {
    VisualNovelPacingPreview preview;
    preview.diagnostics = document.validate();
    preview.auto_advance_enabled = document.controls.auto_advance;
    preview.backlog_entry_count = document.backlog.size();
    preview.unread_entry_count = static_cast<std::size_t>(
        std::count_if(document.backlog.begin(), document.backlog.end(), [](const auto& entry) {
            return !entry.read;
        }));
    preview.skip_available = document.controls.skip_read_text && preview.unread_entry_count < preview.backlog_entry_count;

    const auto visibleCount = std::min(document.controls.backlog_limit, document.backlog.size());
    preview.visible_backlog_count = visibleCount;
    const auto start = document.backlog.size() - visibleCount;
    for (std::size_t index = start; index < document.backlog.size(); ++index) {
        preview.visible_backlog.push_back(document.backlog[index]);
    }

    if (!document.backlog.empty() && document.controls.text_speed_cps > 0.0f) {
        const auto& current = document.backlog.back();
        preview.estimated_line_seconds = static_cast<float>(current.text.size()) / document.controls.text_speed_cps;
        preview.next_advance_seconds = document.controls.auto_advance
            ? preview.estimated_line_seconds + document.controls.auto_advance_delay_seconds
            : 0.0f;
    }

    preview.runtime_commands.push_back("vn_backlog_entries:" + std::to_string(preview.visible_backlog_count));
    preview.runtime_commands.push_back(std::string("vn_auto_advance:") +
                                       (document.controls.auto_advance ? "enabled" : "disabled"));
    preview.runtime_commands.push_back(std::string("vn_skip_read_text:") +
                                       (document.controls.skip_read_text ? "enabled" : "disabled"));
    if (preview.auto_advance_enabled) {
        preview.next_action = "Auto-advance after text reveal and delay.";
    } else if (preview.skip_available) {
        preview.next_action = "Skip already-read backlog entries.";
    } else {
        preview.next_action = "Wait for player advance input.";
    }
    return preview;
}

nlohmann::json visualNovelPacingPreviewToJson(const VisualNovelPacingPreview& preview) {
    nlohmann::json visibleBacklog = nlohmann::json::array();
    for (const auto& entry : preview.visible_backlog) {
        visibleBacklog.push_back(backlogEntryToJson(entry));
    }
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        diagnostics.push_back({
            {"code", diagnostic.code},
            {"target", diagnostic.target},
            {"message", diagnostic.message},
        });
    }
    return {
        {"auto_advance_enabled", preview.auto_advance_enabled},
        {"skip_available", preview.skip_available},
        {"estimated_line_seconds", preview.estimated_line_seconds},
        {"next_advance_seconds", preview.next_advance_seconds},
        {"backlog_entry_count", preview.backlog_entry_count},
        {"visible_backlog_count", preview.visible_backlog_count},
        {"unread_entry_count", preview.unread_entry_count},
        {"visible_backlog", visibleBacklog},
        {"runtime_commands", preview.runtime_commands},
        {"diagnostics", diagnostics},
        {"next_action", preview.next_action},
    };
}

} // namespace urpg::message
