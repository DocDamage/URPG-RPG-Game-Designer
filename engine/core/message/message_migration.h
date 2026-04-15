#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::message {

enum class MessageMigrationSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct MessageMigrationDiagnostic {
    MessageMigrationSeverity severity = MessageMigrationSeverity::Info;
    std::string code;
    std::string page_id;
    std::string message;
    std::string token;
};

struct MessageMigrationResult {
    nlohmann::json dialogue_sequences;
    nlohmann::json message_styles;
    std::vector<MessageMigrationDiagnostic> diagnostics;
    bool used_safe_fallback = false;
};

MessageMigrationResult UpgradeCompatMessageDocument(const nlohmann::json& compat_document);
std::string ExportMessageMigrationDiagnosticsJsonl(const MessageMigrationResult& result);

} // namespace urpg::message
