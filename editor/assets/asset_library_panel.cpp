#include "editor/assets/asset_library_panel.h"

#include <algorithm>
#include <cstring>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#include <shobjidl.h>
#include <windows.h>
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

    const auto package = std::find_if(snapshot.actions.begin(), snapshot.actions.end(), [](const auto& action) {
        return action.id == "package_validate";
    });
    snapshot.package_validation_ready = package != snapshot.actions.end() && package->enabled;
    return snapshot;
}

#ifdef _WIN32

template <typename FunctionPointer>
FunctionPointer loadWindowsProcedure(HMODULE module, const char* name) {
    FARPROC procedure = GetProcAddress(module, name);
    FunctionPointer function = nullptr;
    static_assert(sizeof(function) == sizeof(procedure));
    std::memcpy(&function, &procedure, sizeof(function));
    return function;
}

std::optional<std::filesystem::path> chooseWindowsImportSource(const AssetLibraryPanel::ImportSourcePickerRequest& request) {
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

    const CLSID clsidFileOpenDialog = {0xdc1c5a9c,
                                       0xe88a,
                                       0x4dde,
                                       {0xa5, 0xa1, 0x60, 0xf8, 0x2a, 0x20, 0xae, 0xf7}};
    const IID iidFileOpenDialog = {0xd57c7288,
                                   0xd4ad,
                                   0x4768,
                                   {0xbe, 0x02, 0x9d, 0x96, 0x95, 0x32, 0xd9, 0x60}};

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

std::optional<std::filesystem::path> chooseNativeImportSource(const AssetLibraryPanel::ImportSourcePickerRequest& request) {
#ifdef _WIN32
    return chooseWindowsImportSource(request);
#else
    (void)request;
    return std::nullopt;
#endif
}

} // namespace

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
                                                      const std::filesystem::path& library_root,
                                                      std::string session_id,
                                                      std::string license_note,
                                                      std::vector<std::string> external_extractor_command) {
    auto result = model_.requestImportSource(source, library_root, std::move(session_id), std::move(license_note),
                                             std::move(external_extractor_command));
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::requestImportSourceFromPicker(ImportSourcePickerRequest request) {
    const auto selectedSource =
        import_source_picker_ ? import_source_picker_(request) : chooseNativeImportSource(request);
    if (!selectedSource.has_value() || selectedSource->empty()) {
        nlohmann::json result = {
            {"action", "request_import_source"},
            {"success", false},
            {"code", "import_source_picker_cancelled"},
            {"message", "No import source was selected."},
            {"source_path", ""},
            {"library_root", request.library_root.generic_string()},
            {"session_id", request.session_id},
        };
        refreshRenderSnapshotsFromModel();
        return result;
    }
    return requestImportSource(*selectedSource, request.library_root, std::move(request.session_id),
                               std::move(request.license_note), std::move(request.external_extractor_command));
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
                                                               std::string license_id,
                                                               std::string promoted_root,
                                                               bool include_in_runtime) {
    auto result = model_.promoteImportRecords(std::move(session_id), std::move(asset_ids), std::move(license_id),
                                              std::move(promoted_root), include_in_runtime);
    refreshRenderSnapshotsFromModel();
    return result;
}

nlohmann::json AssetLibraryPanel::attachSelectedPromotedAssetsToProject(std::vector<std::string> paths,
                                                                        const std::filesystem::path& project_root) {
    auto result = model_.attachPromotedAssetsToProject(std::move(paths), project_root);
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
