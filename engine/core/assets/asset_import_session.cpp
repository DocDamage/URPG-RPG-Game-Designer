#include "engine/core/assets/asset_import_session.h"

#include <algorithm>
#include <climits>

namespace urpg::assets {

namespace {

std::vector<std::string> readStringArray(const nlohmann::json& value, const char* key) {
    std::vector<std::string> out;
    const auto it = value.find(key);
    if (it == value.end() || !it->is_array()) {
        return out;
    }
    for (const auto& item : *it) {
        if (item.is_string()) {
            out.push_back(item.get<std::string>());
        }
    }
    return out;
}

std::string readString(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    return it != value.end() && it->is_string() ? it->get<std::string>() : std::string{};
}

size_t readSize(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end()) {
        return 0;
    }
    if (it->is_number_unsigned()) {
        return it->get<size_t>();
    }
    if (it->is_number_integer()) {
        const auto signedValue = it->get<long long>();
        return signedValue > 0 ? static_cast<size_t>(signedValue) : 0;
    }
    return 0;
}

uint64_t readUint64(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end()) {
        return 0;
    }
    if (it->is_number_unsigned()) {
        return it->get<uint64_t>();
    }
    if (it->is_number_integer()) {
        const auto signedValue = it->get<long long>();
        return signedValue > 0 ? static_cast<uint64_t>(signedValue) : 0;
    }
    return 0;
}

int32_t readInt32(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end() || !it->is_number_integer()) {
        return 0;
    }
    const auto signedValue = it->get<long long>();
    return signedValue > 0 ? static_cast<int32_t>(std::min<long long>(signedValue, INT32_MAX)) : 0;
}

AssetImportSummary readSummary(const nlohmann::json& value) {
    AssetImportSummary summary;
    if (!value.is_object()) {
        return summary;
    }
    summary.filesScanned = readSize(value, "filesScanned");
    summary.readyCount = readSize(value, "readyCount");
    summary.needsConversionCount = readSize(value, "needsConversionCount");
    summary.duplicateCount = readSize(value, "duplicateCount");
    summary.missingLicenseCount = readSize(value, "missingLicenseCount");
    summary.unsupportedCount = readSize(value, "unsupportedCount");
    summary.sourceOnlyCount = readSize(value, "sourceOnlyCount");
    summary.errorCount = readSize(value, "errorCount");
    return summary;
}

nlohmann::json toJson(const AssetImportSummary& summary) {
    return {
        {"filesScanned", summary.filesScanned},
        {"readyCount", summary.readyCount},
        {"needsConversionCount", summary.needsConversionCount},
        {"duplicateCount", summary.duplicateCount},
        {"missingLicenseCount", summary.missingLicenseCount},
        {"unsupportedCount", summary.unsupportedCount},
        {"sourceOnlyCount", summary.sourceOnlyCount},
        {"errorCount", summary.errorCount},
    };
}

nlohmann::json toJson(const AssetImportDiagnostic& diagnostic) {
    return {{"code", diagnostic.code}, {"message", diagnostic.message}, {"path", diagnostic.path}};
}

AssetImportDiagnostic diagnosticFromJson(const nlohmann::json& value) {
    AssetImportDiagnostic diagnostic;
    if (!value.is_object()) {
        return diagnostic;
    }
    diagnostic.code = readString(value, "code");
    diagnostic.message = readString(value, "message");
    diagnostic.path = readString(value, "path");
    return diagnostic;
}

std::string reviewState(const AssetImportRecord& record) {
    if (std::find(record.diagnostics.begin(), record.diagnostics.end(), "import_error") != record.diagnostics.end()) {
        return "error";
    }
    if (record.duplicate || !record.duplicateOf.empty()) {
        return "duplicate";
    }
    if (record.toolingOnly || record.sourceOnly) {
        return "source_only";
    }
    if (std::find(record.diagnostics.begin(), record.diagnostics.end(), "unsupported_format") !=
        record.diagnostics.end()) {
        return "unsupported";
    }
    if (record.licenseRequired) {
        return "missing_license";
    }
    if (record.conversionRequired || !record.runtimeReady) {
        return "needs_conversion";
    }
    return "ready_to_promote";
}

std::string recommendedActionForState(const std::string& state) {
    if (state == "ready_to_promote") {
        return "promote";
    }
    if (state == "needs_conversion") {
        return "convert_before_promotion";
    }
    if (state == "duplicate") {
        return "resolve_duplicate";
    }
    if (state == "missing_license") {
        return "add_license_attribution";
    }
    if (state == "unsupported") {
        return "skip_or_replace";
    }
    if (state == "source_only") {
        return "keep_cataloged";
    }
    return "inspect_error";
}

std::string appearanceSlotForCategory(const std::string& category) {
    if (category == "portrait" || category == "character/portrait") {
        return "portrait";
    }
    if (category == "character/field" || category == "field" || category == "sprite") {
        return "field";
    }
    if (category == "character/battle" || category == "battle") {
        return "battle";
    }
    return {};
}

std::string firstBlockedReason(const AssetImportRecord& record, const std::string& state) {
    if (state == "ready_to_promote") {
        return {};
    }
    if (record.licenseRequired) {
        return "license_evidence_missing";
    }
    if (record.normalizedPath.empty()) {
        return "normalized_path_missing";
    }
    if (!record.diagnostics.empty()) {
        return record.diagnostics.front();
    }
    if (state == "needs_conversion") {
        return "source_record_requires_conversion";
    }
    if (state == "duplicate") {
        return "source_record_duplicate";
    }
    if (state == "unsupported") {
        return "source_record_unsupported";
    }
    if (state == "source_only") {
        return "source_record_source_only";
    }
    if (state == "error") {
        return "source_record_import_error";
    }
    return "appearance_part_not_runtime_ready";
}

std::string attributionStateForRecord(const AssetImportRecord& record) {
    return record.licenseRequired ? "missing_license" : "complete";
}

void appendUnique(std::vector<std::string>& diagnostics, const std::string& diagnostic) {
    if (std::find(diagnostics.begin(), diagnostics.end(), diagnostic) == diagnostics.end()) {
        diagnostics.push_back(diagnostic);
    }
}

std::string joinPath(std::string lhs, std::string rhs) {
    std::replace(lhs.begin(), lhs.end(), '\\', '/');
    std::replace(rhs.begin(), rhs.end(), '\\', '/');
    if (lhs.empty()) {
        return rhs;
    }
    if (rhs.empty()) {
        return lhs;
    }
    if (lhs.back() == '/') {
        return lhs + rhs;
    }
    return lhs + "/" + rhs;
}

std::string sanitizePathSegment(std::string value) {
    for (auto& ch : value) {
        const bool keep = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
                          ch == '-' || ch == '_' || ch == '.';
        if (!keep) {
            ch = '-';
        }
    }
    if (value.empty()) {
        return "asset";
    }
    return value;
}

std::string filenameFromRelativePath(const std::string& relativePath) {
    auto normalized = relativePath;
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    const auto slash = normalized.find_last_of('/');
    if (slash == std::string::npos) {
        return normalized.empty() ? "payload" : normalized;
    }
    const auto filename = normalized.substr(slash + 1);
    return filename.empty() ? "payload" : filename;
}

} // namespace

const char* toString(AssetImportSourceKind kind) {
    switch (kind) {
    case AssetImportSourceKind::Folder:
        return "folder";
    case AssetImportSourceKind::File:
        return "file";
    case AssetImportSourceKind::Zip:
        return "zip";
    case AssetImportSourceKind::ExternalArchive:
        return "external_archive";
    case AssetImportSourceKind::UnsupportedArchive:
        return "unsupported_archive";
    }
    return "folder";
}

AssetImportSourceKind assetImportSourceKindFromString(const std::string& value) {
    if (value == "file") {
        return AssetImportSourceKind::File;
    }
    if (value == "zip") {
        return AssetImportSourceKind::Zip;
    }
    if (value == "external_archive") {
        return AssetImportSourceKind::ExternalArchive;
    }
    if (value == "unsupported_archive") {
        return AssetImportSourceKind::UnsupportedArchive;
    }
    return AssetImportSourceKind::Folder;
}

const char* toString(AssetImportStatus status) {
    switch (status) {
    case AssetImportStatus::Pending:
        return "pending";
    case AssetImportStatus::Quarantined:
        return "quarantined";
    case AssetImportStatus::ReviewReady:
        return "review_ready";
    case AssetImportStatus::Failed:
        return "failed";
    }
    return "pending";
}

AssetImportStatus assetImportStatusFromString(const std::string& value) {
    if (value == "quarantined") {
        return AssetImportStatus::Quarantined;
    }
    if (value == "review_ready") {
        return AssetImportStatus::ReviewReady;
    }
    if (value == "failed") {
        return AssetImportStatus::Failed;
    }
    return AssetImportStatus::Pending;
}

nlohmann::json serializeAssetImportSession(const AssetImportSession& session) {
    nlohmann::json records = nlohmann::json::array();
    for (const auto& record : session.records) {
        records.push_back({
            {"assetId", record.assetId},
            {"relativePath", record.relativePath},
            {"normalizedPath", record.normalizedPath},
            {"extension", record.extension},
            {"mediaKind", record.mediaKind},
            {"category", record.category},
            {"pack", record.pack},
            {"sha256", record.sha256},
            {"sizeBytes", record.sizeBytes},
            {"width", record.width},
            {"height", record.height},
            {"durationMs", record.durationMs},
            {"duplicate", record.duplicate},
            {"duplicateOf", record.duplicateOf},
            {"sourceOnly", record.sourceOnly},
            {"toolingOnly", record.toolingOnly},
            {"runtimeReady", record.runtimeReady},
            {"licenseRequired", record.licenseRequired},
            {"diagnostics", record.diagnostics},
            {"conversionRequired", record.conversionRequired},
            {"conversionTargetPath", record.conversionTargetPath},
            {"conversionCommand", record.conversionCommand},
            {"sequenceId", record.sequenceId},
            {"sequenceFrameIndex", record.sequenceFrameIndex},
            {"sequenceFrameCount", record.sequenceFrameCount},
            {"previewAvailable", record.previewAvailable},
            {"previewKind", record.previewKind},
            {"noPreviewDiagnostic", record.noPreviewDiagnostic},
        });
    }

    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : session.diagnostics) {
        diagnostics.push_back(toJson(diagnostic));
    }

    return {
        {"schemaVersion", session.schemaVersion},
        {"sessionId", session.sessionId},
        {"sourceKind", toString(session.sourceKind)},
        {"sourcePath", session.sourcePath},
        {"managedSourceRoot", session.managedSourceRoot},
        {"status", toString(session.status)},
        {"createdAt", session.createdAt},
        {"summary", toJson(session.summary)},
        {"records", records},
        {"diagnostics", diagnostics},
    };
}

AssetImportSession deserializeAssetImportSession(const nlohmann::json& value) {
    AssetImportSession session;
    if (!value.is_object()) {
        return session;
    }
    session.schemaVersion = readString(value, "schemaVersion");
    if (session.schemaVersion.empty()) {
        session.schemaVersion = "1.0.0";
    }
    session.sessionId = readString(value, "sessionId");
    session.sourceKind = assetImportSourceKindFromString(readString(value, "sourceKind"));
    session.sourcePath = readString(value, "sourcePath");
    session.managedSourceRoot = readString(value, "managedSourceRoot");
    session.status = assetImportStatusFromString(readString(value, "status"));
    session.createdAt = readString(value, "createdAt");
    const auto summary = value.find("summary");
    if (summary != value.end()) {
        session.summary = readSummary(*summary);
    }

    const auto records = value.find("records");
    if (records != value.end() && records->is_array()) {
        for (const auto& item : *records) {
            if (!item.is_object()) {
                continue;
            }
            AssetImportRecord record;
            record.assetId = readString(item, "assetId");
            record.relativePath = readString(item, "relativePath");
            record.normalizedPath = readString(item, "normalizedPath");
            record.extension = readString(item, "extension");
            record.mediaKind = readString(item, "mediaKind");
            record.category = readString(item, "category");
            record.pack = readString(item, "pack");
            record.sha256 = readString(item, "sha256");
            record.sizeBytes = readUint64(item, "sizeBytes");
            record.width = readInt32(item, "width");
            record.height = readInt32(item, "height");
            record.durationMs = readInt32(item, "durationMs");
            record.duplicate = item.value("duplicate", false);
            record.duplicateOf = readString(item, "duplicateOf");
            record.sourceOnly = item.value("sourceOnly", false);
            record.toolingOnly = item.value("toolingOnly", false);
            record.runtimeReady = item.value("runtimeReady", false);
            record.licenseRequired = item.value("licenseRequired", true);
            record.diagnostics = readStringArray(item, "diagnostics");
            record.conversionRequired = item.value("conversionRequired", false);
            record.conversionTargetPath = readString(item, "conversionTargetPath");
            record.conversionCommand = readStringArray(item, "conversionCommand");
            record.sequenceId = readString(item, "sequenceId");
            record.sequenceFrameIndex = item.value("sequenceFrameIndex", -1);
            record.sequenceFrameCount = readInt32(item, "sequenceFrameCount");
            record.previewAvailable = item.value("previewAvailable", false);
            record.previewKind = readString(item, "previewKind");
            if (record.previewKind.empty()) {
                record.previewKind = "none";
            }
            record.noPreviewDiagnostic = readString(item, "noPreviewDiagnostic");
            session.records.push_back(std::move(record));
        }
    }

    const auto diagnostics = value.find("diagnostics");
    if (diagnostics != value.end() && diagnostics->is_array()) {
        for (const auto& item : *diagnostics) {
            session.diagnostics.push_back(diagnosticFromJson(item));
        }
    }
    if (session.summary.filesScanned == 0 && !session.records.empty()) {
        session.summary = summarizeAssetImportSession(session);
    }
    return session;
}

AssetImportSummary summarizeAssetImportSession(const AssetImportSession& session) {
    AssetImportSummary summary;
    summary.filesScanned = session.records.size();
    for (const auto& record : session.records) {
        const auto state = reviewState(record);
        if (state == "ready_to_promote") {
            ++summary.readyCount;
        } else if (state == "needs_conversion") {
            ++summary.needsConversionCount;
        } else if (state == "duplicate") {
            ++summary.duplicateCount;
        } else if (state == "missing_license") {
            ++summary.missingLicenseCount;
        } else if (state == "unsupported") {
            ++summary.unsupportedCount;
        } else if (state == "source_only") {
            ++summary.sourceOnlyCount;
        } else if (state == "error") {
            ++summary.errorCount;
        }
    }
    return summary;
}

AssetPromotionManifest planAssetPromotionManifest(const AssetImportSession& session, const AssetImportRecord& record,
                                                  std::string licenseId, std::string promotedRoot,
                                                  bool includeInRuntime) {
    AssetPromotionManifest manifest;
    manifest.assetId = record.assetId;
    manifest.sourcePath = joinPath(session.managedSourceRoot, record.relativePath);
    manifest.licenseId = std::move(licenseId);
    manifest.promotedPath =
        joinPath(joinPath(std::move(promotedRoot), sanitizePathSegment(record.assetId)), "payloads/" +
                                                                                          filenameFromRelativePath(record.relativePath));
    manifest.preview.kind = record.mediaKind == "image" || record.mediaKind == "audio" ? record.mediaKind : "none";
    manifest.preview.thumbnailPath = manifest.preview.kind == "none" ? "" : manifest.sourcePath;
    manifest.preview.width = record.width;
    manifest.preview.height = record.height;
    manifest.package.includeInRuntime = includeInRuntime;
    manifest.package.requiredForRelease = false;
    manifest.diagnostics = record.diagnostics;

    const auto state = reviewState(record);
    if (state == "ready_to_promote") {
        manifest.status = AssetPromotionStatus::RuntimeReady;
    } else {
        manifest.status = AssetPromotionStatus::Blocked;
        manifest.package.includeInRuntime = false;
        if (state == "needs_conversion") {
            appendUnique(manifest.diagnostics, "source_record_requires_conversion");
        } else if (state == "duplicate") {
            appendUnique(manifest.diagnostics, "source_record_duplicate");
        } else if (state == "missing_license") {
            appendUnique(manifest.diagnostics, "license_evidence_missing");
        } else if (state == "unsupported") {
            appendUnique(manifest.diagnostics, "source_record_unsupported");
        } else if (state == "source_only") {
            appendUnique(manifest.diagnostics, "source_record_source_only");
        } else if (state == "error") {
            appendUnique(manifest.diagnostics, "source_record_import_error");
        }
    }

    if (manifest.licenseId.empty()) {
        manifest.status = AssetPromotionStatus::Blocked;
        manifest.package.includeInRuntime = false;
        appendUnique(manifest.diagnostics, "license_evidence_missing");
    }
    if (record.normalizedPath.empty()) {
        manifest.status = AssetPromotionStatus::Blocked;
        manifest.package.includeInRuntime = false;
        appendUnique(manifest.diagnostics, "normalized_path_missing");
    }
    if (manifest.status == AssetPromotionStatus::Blocked) {
        manifest.promotedPath.clear();
        if (manifest.preview.kind != "image" && manifest.preview.kind != "audio") {
            manifest.preview.kind = "pending";
        }
    }
    for (const auto& diagnostic : validateAssetPromotionManifest(manifest)) {
        appendUnique(manifest.diagnostics, diagnostic);
    }
    return manifest;
}

nlohmann::json buildAssetImportSessionRows(const std::vector<AssetImportSession>& sessions) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& session : sessions) {
        const auto summary = session.summary.filesScanned > 0 ? session.summary : summarizeAssetImportSession(session);
        rows.push_back({
            {"session_id", session.sessionId},
            {"source_kind", toString(session.sourceKind)},
            {"source_path", session.sourcePath},
            {"managed_source_root", session.managedSourceRoot},
            {"status", toString(session.status)},
            {"created_at", session.createdAt},
            {"summary", toJson(summary)},
            {"diagnostic_count", session.diagnostics.size()},
        });
    }
    std::sort(rows.begin(), rows.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.value("created_at", "") < rhs.value("created_at", "");
    });
    return rows;
}

nlohmann::json buildAssetImportReviewRows(const std::vector<AssetImportSession>& sessions) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& session : sessions) {
        for (const auto& record : session.records) {
            const auto state = reviewState(record);
            rows.push_back({
                {"session_id", session.sessionId},
                {"asset_id", record.assetId},
                {"relative_path", record.relativePath},
                {"normalized_path", record.normalizedPath},
                {"extension", record.extension},
                {"media_kind", record.mediaKind},
                {"category", record.category},
                {"pack", record.pack},
                {"sha256", record.sha256},
                {"size_bytes", record.sizeBytes},
                {"width", record.width},
                {"height", record.height},
                {"duration_ms", record.durationMs},
                {"duplicate_of", record.duplicateOf},
                {"review_state", state},
                {"recommended_action", recommendedActionForState(state)},
                {"promotable", state == "ready_to_promote"},
                {"diagnostics", record.diagnostics},
                {"conversion_required", record.conversionRequired},
                {"conversion_target_path", record.conversionTargetPath},
                {"conversion_command", record.conversionCommand},
                {"sequence_id", record.sequenceId},
                {"sequence_frame_index", record.sequenceFrameIndex},
                {"sequence_frame_count", record.sequenceFrameCount},
                {"preview_available", record.previewAvailable},
                {"preview_kind", record.previewKind},
                {"no_preview_diagnostic", record.noPreviewDiagnostic},
            });
        }
    }
    std::sort(rows.begin(), rows.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.value("session_id", "") != rhs.value("session_id", "")) {
            return lhs.value("session_id", "") < rhs.value("session_id", "");
        }
        return lhs.value("relative_path", "") < rhs.value("relative_path", "");
    });
    return rows;
}

nlohmann::json buildAppearancePartImportRows(const std::vector<AssetImportSession>& sessions) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& session : sessions) {
        for (const auto& record : session.records) {
            const auto slot = appearanceSlotForCategory(record.category);
            if (slot.empty()) {
                continue;
            }
            const auto state = reviewState(record);
            const auto blockedReason = firstBlockedReason(record, state);
            const bool runtimeReady = state == "ready_to_promote" && blockedReason.empty();
            const auto sourcePath = joinPath(session.managedSourceRoot, record.relativePath);
            rows.push_back({
                {"session_id", session.sessionId},
                {"asset_id", record.assetId},
                {"normalized_asset_id", record.assetId},
                {"relative_path", record.relativePath},
                {"source_path", sourcePath},
                {"normalized_path", record.normalizedPath},
                {"slot", slot},
                {"category", record.category},
                {"media_kind", record.mediaKind},
                {"pack", record.pack},
                {"sha256", record.sha256},
                {"size_bytes", record.sizeBytes},
                {"dimensions", {{"width", record.width}, {"height", record.height}}},
                {"runtime_ready", runtimeReady},
                {"attribution_state", attributionStateForRecord(record)},
                {"blocked_reason", runtimeReady ? nlohmann::json(nullptr) : nlohmann::json(blockedReason)},
                {"diagnostics", record.diagnostics},
                {"preview", {{"available", record.previewAvailable}, {"kind", record.previewKind}}},
                {"management_actions",
                 {
                     {"accept",
                      {{"enabled", runtimeReady},
                       {"action", "accept_appearance_part"},
                       {"disabled_reason",
                        runtimeReady ? nlohmann::json(nullptr) : nlohmann::json(blockedReason)}}},
                     {"reject",
                      {{"enabled", true},
                       {"action", "reject_appearance_part"},
                       {"disabled_reason", nlohmann::json(nullptr)}}},
                     {"archive",
                      {{"enabled", false},
                       {"action", "archive_appearance_part"},
                       {"disabled_reason", "not_promoted"}}},
                     {"assign",
                      {{"enabled", runtimeReady},
                       {"action", "assign_appearance_part"},
                       {"target_slot", slot},
                       {"disabled_reason",
                        runtimeReady ? nlohmann::json(nullptr) : nlohmann::json(blockedReason)}}},
                 }},
            });
        }
    }
    std::sort(rows.begin(), rows.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.value("slot", "") != rhs.value("slot", "")) {
            return lhs.value("slot", "") < rhs.value("slot", "");
        }
        return lhs.value("asset_id", "") < rhs.value("asset_id", "");
    });
    return rows;
}

} // namespace urpg::assets
