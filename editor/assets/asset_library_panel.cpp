#include "editor/assets/asset_library_panel.h"
#include "engine/core/platform/process_runner.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <system_error>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#include <shobjidl.h>
#include <windows.h>
#endif

#ifdef __APPLE__
extern "C" bool urpgChooseMacOSImportSource(bool folder, char* output, size_t outputSize);
#endif

namespace urpg::editor {

namespace {

AssetLibraryPanel::ImportWizardRenderSnapshot buildImportWizardRenderSnapshot(const nlohmann::json& wizard) {
    AssetLibraryPanel::ImportWizardRenderSnapshot snapshot;
    if (!wizard.is_object()) {
        return snapshot;
    }

    snapshot.status = wizard.value("status", snapshot.status);
    snapshot.current_step = wizard.value("current_step", snapshot.current_step);
    snapshot.extractor_configuration =
        wizard.contains("extractor_configuration") ? wizard["extractor_configuration"] : nlohmann::json::object();
    snapshot.pending_request = wizard.contains("pending_request") ? wizard["pending_request"] : nlohmann::json(nullptr);

    if (wizard.contains("steps") && wizard["steps"].is_array()) {
        for (const auto& step : wizard["steps"]) {
            AssetLibraryPanel::ImportWizardStepSnapshot row;
            row.id = step.value("id", "");
            row.label = step.value("label", "");
            row.state = step.value("state", "");
            row.count = step.value("count", 0u);
            row.active = row.state == "active";
            row.complete = row.state == "complete";
            row.available = row.state == "available";
            snapshot.steps.push_back(std::move(row));
        }
    }

    if (wizard.contains("actions") && wizard["actions"].is_object()) {
        for (const auto& [id, action] : wizard["actions"].items()) {
            AssetLibraryPanel::ImportWizardActionSnapshot row;
            row.id = id;
            row.action = action.value("action", "");
            row.enabled = action.value("enabled", false);
            row.pending_request = action.value("pending_request", false);
            row.eligible_count = action.value("eligible_count", 0u);
            if (action.contains("disabled_reason") && action["disabled_reason"].is_string()) {
                row.disabled_reason = action["disabled_reason"].get<std::string>();
            }
            snapshot.actions.push_back(std::move(row));
        }
    }

    const auto package = std::find_if(snapshot.actions.begin(), snapshot.actions.end(),
                                      [](const auto& action) { return action.id == "package_validate"; });
    snapshot.package_validation_ready = package != snapshot.actions.end() && package->enabled;
    return snapshot;
}

#ifdef _WIN32

template<typename FunctionPointer> FunctionPointer loadWindowsProcedure(HMODULE module, const char* name) {
    FARPROC procedure = GetProcAddress(module, name);
    FunctionPointer function = nullptr;
    static_assert(sizeof(function) == sizeof(procedure));
    std::memcpy(&function, &procedure, sizeof(function));
    return function;
}

std::optional<std::filesystem::path>
chooseWindowsImportSource(const AssetLibraryPanel::ImportSourcePickerRequest& request) {
    using CoInitializeExFn = HRESULT(WINAPI*)(LPVOID, DWORD);
    using CoUninitializeFn = void(WINAPI*)();
    using CoCreateInstanceFn = HRESULT(WINAPI*)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
    using CoTaskMemFreeFn = void(WINAPI*)(LPVOID);

    HMODULE ole32 = LoadLibraryW(L"ole32.dll");
    if (ole32 == nullptr) {
        return std::nullopt;
    }

    const auto coInitializeEx = loadWindowsProcedure<CoInitializeExFn>(ole32, "CoInitializeEx");
    const auto coUninitialize = loadWindowsProcedure<CoUninitializeFn>(ole32, "CoUninitialize");
    const auto coCreateInstance = loadWindowsProcedure<CoCreateInstanceFn>(ole32, "CoCreateInstance");
    const auto coTaskMemFree = loadWindowsProcedure<CoTaskMemFreeFn>(ole32, "CoTaskMemFree");
    if (coInitializeEx == nullptr || coUninitialize == nullptr || coCreateInstance == nullptr ||
        coTaskMemFree == nullptr) {
        FreeLibrary(ole32);
        return std::nullopt;
    }

    const HRESULT initResult = coInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool shouldUninitialize = SUCCEEDED(initResult);
    if (FAILED(initResult) && initResult != RPC_E_CHANGED_MODE) {
        FreeLibrary(ole32);
        return std::nullopt;
    }

    const CLSID clsidFileOpenDialog = {0xdc1c5a9c, 0xe88a, 0x4dde, {0xa5, 0xa1, 0x60, 0xf8, 0x2a, 0x20, 0xae, 0xf7}};
    const IID iidFileOpenDialog = {0xd57c7288, 0xd4ad, 0x4768, {0xbe, 0x02, 0x9d, 0x96, 0x95, 0x32, 0xd9, 0x60}};

    IFileOpenDialog* dialog = nullptr;
    if (FAILED(coCreateInstance(clsidFileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, iidFileOpenDialog,
                                reinterpret_cast<void**>(&dialog))) ||
        dialog == nullptr) {
        if (shouldUninitialize) {
            coUninitialize();
        }
        FreeLibrary(ole32);
        return std::nullopt;
    }

    DWORD options = 0;
    if (SUCCEEDED(dialog->GetOptions(&options))) {
        options |= FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;
        if (request.mode == AssetLibraryPanel::ImportSourcePickerMode::Folder) {
            options |= FOS_PICKFOLDERS;
        } else {
            options |= FOS_FILEMUSTEXIST;
        }
        dialog->SetOptions(options);
    }
    dialog->SetTitle(request.mode == AssetLibraryPanel::ImportSourcePickerMode::Folder
                         ? L"Choose Asset Source Folder"
                         : L"Choose Asset Source File or Archive");

    std::optional<std::filesystem::path> selected;
    if (SUCCEEDED(dialog->Show(nullptr))) {
        IShellItem* item = nullptr;
        if (SUCCEEDED(dialog->GetResult(&item)) && item != nullptr) {
            PWSTR selectedPath = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &selectedPath)) && selectedPath != nullptr) {
                selected = std::filesystem::path(selectedPath);
                coTaskMemFree(selectedPath);
            }
            item->Release();
        }
    }

    dialog->Release();
    if (shouldUninitialize) {
        coUninitialize();
    }
    FreeLibrary(ole32);
    return selected;
}

#endif

#ifdef __linux__

std::vector<std::filesystem::path> splitLinuxPathList(const char* pathList) {
    std::vector<std::filesystem::path> paths;
    if (pathList == nullptr || *pathList == '\0') {
        return paths;
    }

    std::string value(pathList);
    size_t begin = 0;
    while (begin <= value.size()) {
        const size_t end = value.find(':', begin);
        const auto part = value.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
        if (!part.empty()) {
            paths.emplace_back(part);
        }
        if (end == std::string::npos) {
            break;
        }
        begin = end + 1;
    }
    return paths;
}

bool executableExistsOnPath(const std::string& executable) {
    std::error_code ec;
    for (const auto& root : splitLinuxPathList(std::getenv("PATH"))) {
        const auto candidate = root / executable;
        if (std::filesystem::exists(candidate, ec) && !ec) {
            return true;
        }
        ec.clear();
    }
    return false;
}

bool linuxDesktopPortalAvailable() {
    const char* sessionBus = std::getenv("DBUS_SESSION_BUS_ADDRESS");
    return sessionBus != nullptr && *sessionBus != '\0' && executableExistsOnPath("gdbus");
}

std::optional<std::string> linuxDesktopHelper() {
    if (executableExistsOnPath("zenity")) {
        return "zenity";
    }
    if (executableExistsOnPath("kdialog")) {
        return "kdialog";
    }
    return std::nullopt;
}

std::string trimPickerOutput(std::string value) {
    while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
        value.pop_back();
    }
    return value;
}

std::optional<std::filesystem::path>
chooseLinuxDesktopHelperImportSource(const AssetLibraryPanel::ImportSourcePickerRequest& request,
                                     const std::string& executable) {
    std::vector<std::string> arguments;
    const bool folder = request.mode == AssetLibraryPanel::ImportSourcePickerMode::Folder;
    if (executable == "zenity") {
        arguments = {"--file-selection",
                     "--title",
                     folder ? "Choose Asset Source Folder" : "Choose Asset Source File or Archive"};
        if (folder) {
            arguments.push_back("--directory");
        }
    } else if (executable == "kdialog") {
        arguments = {"--title", folder ? "Choose Asset Source Folder" : "Choose Asset Source File or Archive",
                     folder ? "--getexistingdirectory" : "--getopenfilename", ""};
    } else {
        return std::nullopt;
    }

    urpg::platform::ProcessCommand command;
    command.executable = executable;
    command.arguments = std::move(arguments);
    command.timeout = std::chrono::minutes(60);
    command.captureStdout = true;
    command.captureStderr = true;
    const auto result = urpg::platform::runProcess(command);
    if (result.exitCode != 0 || result.timedOut) {
        return std::nullopt;
    }

    auto selected = trimPickerOutput(result.stdoutText);
    return selected.empty() ? std::nullopt : std::optional<std::filesystem::path>{std::filesystem::path(selected)};
}

std::optional<std::filesystem::path>
chooseLinuxImportSource(const AssetLibraryPanel::ImportSourcePickerRequest& request) {
    // xdg-desktop-portal is detected first for diagnostics. Dependency-free builds fall through to common
    // desktop helpers for the synchronous selected path payload used by the import wizard.
    (void)linuxDesktopPortalAvailable();
    const auto helper = linuxDesktopHelper();
    if (!helper.has_value()) {
        return std::nullopt;
    }
    return chooseLinuxDesktopHelperImportSource(request, *helper);
}

#endif

std::optional<std::filesystem::path>
chooseNativeImportSource(const AssetLibraryPanel::ImportSourcePickerRequest& request) {
#ifdef _WIN32
    return chooseWindowsImportSource(request);
#elif defined(__APPLE__)
    char selected[4096] = {};
    const bool folder = request.mode == AssetLibraryPanel::ImportSourcePickerMode::Folder;
    return urpgChooseMacOSImportSource(folder, selected, sizeof(selected))
               ? std::optional<std::filesystem::path>{std::filesystem::path(selected)}
               : std::nullopt;
#elif defined(__linux__)
    return chooseLinuxImportSource(request);
#else
    (void)request;
    return std::nullopt;
#endif
}

std::string validateSelectedImportSource(const std::filesystem::path& source,
                                         AssetLibraryPanel::ImportSourcePickerMode mode) {
    if (source.empty()) {
        return "empty_source_path";
    }

    std::error_code ec;
    const bool exists = std::filesystem::exists(source, ec);
    if (ec || !exists) {
        return "source_path_not_found";
    }

    if (mode == AssetLibraryPanel::ImportSourcePickerMode::Folder) {
        return std::filesystem::is_directory(source, ec) && !ec ? "" : "source_path_not_folder";
    }

    return (std::filesystem::is_regular_file(source, ec) || std::filesystem::is_directory(source, ec)) && !ec
               ? ""
               : "source_path_not_importable";
}

} // namespace

AssetLibraryPanel::ImportSourcePickerAvailability AssetLibraryPanel::nativeImportSourcePickerAvailability() {
#ifdef _WIN32
    return nativeImportSourcePickerAvailabilityForDiagnostics(NativeImportSourcePickerPlatform::Windows, false, false);
#elif defined(__APPLE__)
    return nativeImportSourcePickerAvailabilityForDiagnostics(NativeImportSourcePickerPlatform::MacOS, false, false);
#elif defined(__linux__)
    return nativeImportSourcePickerAvailabilityForDiagnostics(NativeImportSourcePickerPlatform::Linux,
                                                             linuxDesktopPortalAvailable(),
                                                             linuxDesktopHelper().has_value());
#else
    return nativeImportSourcePickerAvailabilityForDiagnostics(NativeImportSourcePickerPlatform::Unsupported, false,
                                                             false);
#endif
}

AssetLibraryPanel::ImportSourcePickerAvailability
AssetLibraryPanel::nativeImportSourcePickerAvailabilityForDiagnostics(NativeImportSourcePickerPlatform platform,
                                                                      bool desktop_portal_available,
                                                                      bool desktop_helper_available) {
    switch (platform) {
    case NativeImportSourcePickerPlatform::Windows:
        return {true, true, "native_import_source_picker_available",
                "Native Windows import source picker is available."};
    case NativeImportSourcePickerPlatform::MacOS:
        return {true, true, "native_import_source_picker_available",
                "Native macOS import source picker is available."};
    case NativeImportSourcePickerPlatform::Linux:
        if (desktop_portal_available || desktop_helper_available) {
            return {true, true, "native_import_source_picker_available",
                    desktop_portal_available
                        ? "Linux desktop portal import source picker is available."
                        : "Linux desktop helper import source picker is available; xdg-desktop-portal is unavailable."};
        }
        return {false, true, "native_import_source_picker_portal_missing",
                "Linux native import source picker needs xdg-desktop-portal or a supported desktop helper; use path "
                "entry import instead."};
    case NativeImportSourcePickerPlatform::Unsupported:
        break;
    }
    return {false, true, "native_import_source_picker_unsupported",
            "Native import source picker is not implemented on this platform; use path entry import instead."};
}

void AssetLibraryPanel::setImportSourcePicker(ImportSourcePicker picker) {
    import_source_picker_ = std::move(picker);
}

void AssetLibraryPanel::render() {
    if (!visible_) {
        return;
    }
    refreshRenderSnapshotsFromModel();
    has_rendered_frame_ = true;
}

nlohmann::json AssetLibraryPanel::requestImportSource(const std::filesystem::path& source,
                                                      const std::filesystem::path& library_root, std::string session_id,
                                                      std::string license_note,
                                                      std::vector<std::string> external_extractor_command,
                                                      std::vector<std::string> selected_archive_entries) {
    auto result = model_.requestImportSource(source, library_root, std::move(session_id), std::move(license_note),
                                             std::move(external_extractor_command), std::move(selected_archive_entries));
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::requestImportSourceFromPicker(ImportSourcePickerRequest request) {
    const bool usingNativePicker = !import_source_picker_;
    const auto selectedSource =
        import_source_picker_ ? import_source_picker_(request) : chooseNativeImportSource(request);
    if (!selectedSource.has_value() || selectedSource->empty()) {
        const auto availability = nativeImportSourcePickerAvailability();
        const bool unsupported = usingNativePicker && !availability.available;
        nlohmann::json result = {
            {"action", "request_import_source"},
            {"success", false},
            {"code", unsupported ? "import_source_picker_unsupported" : "import_source_picker_cancelled"},
            {"message", unsupported ? "Native import source picker is unavailable; enter a source path manually."
                                    : "No import source was selected."},
            {"source_path", ""},
            {"library_root", request.library_root.generic_string()},
            {"session_id", request.session_id},
            {"picker_availability",
             {
                 {"available", availability.available},
                 {"path_entry_available", availability.path_entry_available},
                 {"code", availability.code},
                 {"message", availability.message},
             }},
        };
        refreshRenderSnapshotsFromModel();
        return result;
    }

    const auto invalidReason = validateSelectedImportSource(*selectedSource, request.mode);
    if (!invalidReason.empty()) {
        const auto availability = nativeImportSourcePickerAvailability();
        nlohmann::json result = {
            {"action", "request_import_source"},
            {"success", false},
            {"code", "import_source_picker_invalid_path"},
            {"message", "Selected import source path is not usable."},
            {"source_path", selectedSource->generic_string()},
            {"library_root", request.library_root.generic_string()},
            {"session_id", request.session_id},
            {"invalid_reason", invalidReason},
            {"picker_availability",
             {
                 {"available", availability.available},
                 {"path_entry_available", availability.path_entry_available},
                 {"code", availability.code},
                 {"message", availability.message},
             }},
        };
        refreshRenderSnapshotsFromModel();
        return result;
    }

    return requestImportSource(*selectedSource, request.library_root, std::move(request.session_id),
                               std::move(request.license_note), std::move(request.external_extractor_command));
}

nlohmann::json AssetLibraryPanel::executePendingImportRequest(AssetLibraryModel::ConversionCommandExecutor executor) {
    auto result = model_.executePendingImportRequest(std::move(executor));
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::refreshExternalCatalog(AssetLibraryModel::ConversionCommandExecutor executor) {
    auto result = model_.refreshExternalCatalog(std::move(executor));
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json
AssetLibraryPanel::openSelectedExternalCatalogSource(AssetLibraryModel::ConversionCommandExecutor executor) {
    auto result = model_.openSelectedExternalCatalogSource(std::move(executor));
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::convertSelectedImportRecords(std::string session_id,
                                                               std::vector<std::string> asset_ids,
                                                               AssetLibraryModel::ConversionCommandExecutor executor) {
    auto result = model_.runImportRecordConversions(std::move(session_id), std::move(asset_ids), std::move(executor));
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::promoteSelectedImportRecords(std::string session_id,
                                                               std::vector<std::string> asset_ids,
                                                               std::string license_id, std::string promoted_root,
                                                               bool include_in_runtime) {
    auto result = model_.promoteImportRecords(std::move(session_id), std::move(asset_ids), std::move(license_id),
                                              std::move(promoted_root), include_in_runtime);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::planPromotedAssetAttachmentToProject(
    std::string path, const std::filesystem::path& project_root,
    const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    auto result = model_.planPromotedAssetAttachmentToProject(std::move(path), project_root, policy);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::confirmPromotedAssetAttachmentToProject(
    std::string path, const std::filesystem::path& project_root, std::string expected_source_revision,
    std::string operation_id, const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    auto result = model_.confirmPromotedAssetAttachmentToProject(std::move(path), project_root,
                                                                  std::move(expected_source_revision),
                                                                  std::move(operation_id), policy);
    refreshRenderSnapshotsFromModel();
    return result.toJson();
}

nlohmann::json AssetLibraryPanel::planDerivedRevisionAttachmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    auto result = model_.planDerivedRevisionAttachmentToProject(std::move(source_path), derived_manifest_path,
                                                                 project_root, policy);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::confirmDerivedRevisionAttachmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, std::string expected_source_revision, std::string operation_id,
    const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    auto result = model_.confirmDerivedRevisionAttachmentToProject(
        std::move(source_path), derived_manifest_path, project_root, std::move(expected_source_revision),
        std::move(operation_id), policy);
    refreshRenderSnapshotsFromModel();
    return result.toJson();
}

nlohmann::json AssetLibraryPanel::recoverDerivedAttachmentReference(
    const std::filesystem::path& derived_manifest_path, const std::filesystem::path& project_root) {
    auto result = model_.recoverDerivedAttachmentReference(derived_manifest_path, project_root);
    refreshRenderSnapshotsFromModel();
    return result.toJson();
}

nlohmann::json AssetLibraryPanel::planDerivedTilesetAssignmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    auto result = model_.planDerivedTilesetAssignmentToProject(std::move(source_path), derived_manifest_path,
                                                                 project_root, policy);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::confirmDerivedTilesetAssignmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, std::string expected_source_revision, std::string operation_id,
    const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    auto result = model_.confirmDerivedTilesetAssignmentToProject(
        std::move(source_path), derived_manifest_path, project_root, std::move(expected_source_revision),
        std::move(operation_id), policy);
    refreshRenderSnapshotsFromModel();
    return result.toJson();
}

nlohmann::json AssetLibraryPanel::createImageCropScaleRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const int32_t crop_x, const int32_t crop_y, const int32_t crop_width, const int32_t crop_height,
    const int32_t output_width, const int32_t output_height) {
    auto result = model_.createImageCropScaleRevision(std::move(source_path), derived_root, std::move(operation_id),
                                                       crop_x, crop_y, crop_width, crop_height, output_width,
                                                       output_height);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::createImagePaletteRevision(std::string source_path,
                                                              const std::filesystem::path& derived_root,
                                                              std::string operation_id,
                                                              std::vector<uint32_t> colors_rgba, const bool dither) {
    auto result = model_.createImagePaletteRevision(std::move(source_path), derived_root, std::move(operation_id),
                                                    std::move(colors_rgba), dither);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::createAudioTrimFadeGainRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const uint64_t start_frame, const uint64_t end_frame, const uint64_t fade_in_frames,
    const uint64_t fade_out_frames, const int32_t gain_milli_db, const int64_t loop_start_frame,
    const int64_t loop_end_frame) {
    auto result = model_.createAudioTrimFadeGainRevision(
        std::move(source_path), derived_root, std::move(operation_id), start_frame, end_frame, fade_in_frames,
        fade_out_frames, gain_milli_db, loop_start_frame, loop_end_frame);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::createTilesetSliceRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const int32_t tile_width, const int32_t tile_height, const int32_t margin, const int32_t spacing) {
    auto result = model_.createTilesetSliceRevision(std::move(source_path), derived_root, std::move(operation_id),
                                                    tile_width, tile_height, margin, spacing);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::createAtlasMetadataRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const int32_t atlas_width, const int32_t atlas_height, const int32_t frame_width, const int32_t frame_height) {
    auto result = model_.createAtlasMetadataRevision(std::move(source_path), derived_root, std::move(operation_id),
                                                     atlas_width, atlas_height, frame_width, frame_height);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::attachSelectedPromotedAssetsToProject(std::vector<std::string> paths,
                                                                        const std::filesystem::path& project_root,
                                                                        const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    auto result = model_.attachPromotedAssetsToProject(std::move(paths), project_root, policy);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::browseArchive(const std::filesystem::path& archive_path) {
    auto result = model_.browseArchive(archive_path);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::validatePackage(const urpg::tools::ExportConfig& config) {
    refreshRenderSnapshotsFromModel();
    if (!last_import_wizard_snapshot_.package_validation_ready) {
        return {
            {"action", "asset_library_package_validate"},
            {"success", false},
            {"code", "package_validation_not_ready"},
            {"message", "Attach promoted project assets before running package validation."},
            {"errors", nlohmann::json::array({"no_attached_project_assets"})},
        };
    }

    urpg::tools::ExportPackager packager;
    const auto validation = packager.validateBeforeExport(config);
    return {
        {"action", "asset_library_package_validate"},
        {"success", validation.passed},
        {"code", validation.passed ? "package_validation_passed" : "package_validation_failed"},
        {"message", validation.passed ? "Package validation passed." : "Package validation failed."},
        {"errors", validation.errors},
    };
}

void AssetLibraryPanel::refreshRenderSnapshotsFromModel() {
    last_render_snapshot_ = model_.snapshot();
    last_import_wizard_snapshot_ = buildImportWizardRenderSnapshot(last_render_snapshot_.import_wizard);
}

} // namespace urpg::editor
