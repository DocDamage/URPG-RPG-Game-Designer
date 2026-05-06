#include "editor/assets/asset_library_panel.h"

#include <algorithm>
#include <cstring>
#include <system_error>
#include <utility>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

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

#ifdef URPG_IMGUI_ENABLED
std::string jsonCellString(const nlohmann::json& row, const char* key) {
    if (!row.is_object() || !row.contains(key) || row[key].is_null()) {
        return "-";
    }
    if (row[key].is_string()) {
        return row[key].get<std::string>();
    }
    if (row[key].is_boolean()) {
        return row[key].get<bool>() ? "yes" : "no";
    }
    if (row[key].is_number_integer()) {
        return std::to_string(row[key].get<long long>());
    }
    if (row[key].is_number_unsigned()) {
        return std::to_string(row[key].get<unsigned long long>());
    }
    if (row[key].is_number_float()) {
        return std::to_string(row[key].get<double>());
    }
    return row[key].dump();
}

void renderJsonRows(const char* tableId, const nlohmann::json& rows, std::initializer_list<const char*> columns,
                    int maxRows = 12) {
    if (!rows.is_array() || rows.empty()) {
        ImGui::TextDisabled("No rows.");
        return;
    }

    if (!ImGui::BeginTable(tableId, static_cast<int>(columns.size()),
                           ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                               ImGuiTableFlags_ScrollY,
                           ImVec2(0.0f, 180.0f))) {
        return;
    }

    for (const auto* column : columns) {
        ImGui::TableSetupColumn(column);
    }
    ImGui::TableHeadersRow();

    int rendered = 0;
    for (const auto& row : rows) {
        if (rendered++ >= maxRows) {
            break;
        }
        ImGui::TableNextRow();
        int columnIndex = 0;
        for (const auto* column : columns) {
            ImGui::TableSetColumnIndex(columnIndex++);
            const auto cell = jsonCellString(row, column);
            ImGui::TextUnformatted(cell.c_str());
        }
    }

    ImGui::EndTable();
}

void renderAssetBrowserDrawer(const nlohmann::json& browser) {
    const auto visibleCount = browser.value("visible_count", size_t{0});
    const auto totalCount = browser.value("total_count", size_t{0});
    ImGui::Text("Rows: %zu / %zu", visibleCount, totalCount);
    ImGui::Text("Query: %s", browser.value("query", "").c_str());
    if (browser.contains("filters")) {
        ImGui::Text("Pack: %s", browser["filters"].value("pack", "").c_str());
        ImGui::Text("Category: %s", browser["filters"].value("category", "").c_str());
        ImGui::Text("Source: %s", browser["filters"].value("source", "").c_str());
    }

    ImGui::SeparatorText("Folders");
    renderJsonRows("AssetBrowserDrawerFolders", browser.value("folder_tree", nlohmann::json::array()), {"id", "count"},
                   8);
    ImGui::SeparatorText("Categories");
    renderJsonRows("AssetBrowserDrawerCategories", browser.value("category_tree", nlohmann::json::array()),
                   {"id", "count"}, 8);
    ImGui::SeparatorText("Packs");
    renderJsonRows("AssetBrowserDrawerPacks", browser.value("pack_tree", nlohmann::json::array()), {"id", "count"},
                   8);
}

bool renderAssetBrowserRowsAndPreview(const nlohmann::json& browser) {
    bool useSelectedInLevelBuilder = false;
    ImGui::SeparatorText("Assets");
    renderJsonRows("AssetBrowserDrawerRows", browser.value("visible_rows", nlohmann::json::array()),
                   {"stableId", "displayName", "mediaKind", "category", "pack"}, 16);

    const auto selected = browser.contains("selected_record") ? browser["selected_record"] : nlohmann::json::object();
    if (selected.is_object() && !selected.empty()) {
        ImGui::SeparatorText("Preview");
        ImGui::Text("Name: %s", selected.value("displayName", "").c_str());
        ImGui::Text("Path: %s", selected.value("sourcePath", "").c_str());
        ImGui::Text("Kind: %s / %s", selected.value("previewKind", "").c_str(),
                    selected.value("mediaKind", "").c_str());
        if (selected.contains("dimensions") && selected["dimensions"].is_object()) {
            ImGui::Text("Size: %d x %d",
                        selected["dimensions"].value("width", 0),
                        selected["dimensions"].value("height", 0));
        }
        useSelectedInLevelBuilder = ImGui::Button("Use in Level Builder", ImVec2(-1.0f, 0.0f));
    }
    return useSelectedInLevelBuilder;
}

void renderAssetLibraryWindow(const AssetLibraryModelSnapshot& snapshot,
                              const AssetLibraryPanel::ImportWizardRenderSnapshot& wizard,
                              bool* useSelectedInLevelBuilder) {
    if (!ImGui::Begin("Assets")) {
        ImGui::End();
        return;
    }

    ImGui::Text("Status: %s", snapshot.status.c_str());
    ImGui::SameLine(220.0f);
    ImGui::Text("Export eligible: %s", snapshot.export_eligible ? "yes" : "no");
    ImGui::TextWrapped("%s", snapshot.status_message.c_str());
    if (!snapshot.error_message.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "%s", snapshot.error_message.c_str());
    }
    ImGui::Separator();

    if (ImGui::BeginTable("AssetMetrics", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Assets");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%zu", snapshot.asset_count);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Runtime ready");
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%zu", snapshot.runtime_ready_count);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Issues");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%zu", snapshot.issue_count);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Previewable");
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%zu", snapshot.previewable_count);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Import sessions");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%zu", snapshot.import_session_count);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("Import ready");
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%zu", snapshot.import_ready_count);
        ImGui::EndTable();
    }

    const auto& browser = snapshot.asset_index_browser;
    const bool browserAvailable = browser.is_object() &&
                                  (browser.value("visible_count", size_t{0}) > 0 ||
                                   browser.value("total_count", size_t{0}) > 0);
    const bool showLeftDrawer = browserAvailable &&
                                snapshot.asset_browser_scope.value("browser_layout", "") ==
                                    "left_collapsible_folder_tree";
    if (showLeftDrawer) {
        const float drawerWidth = std::min(360.0f, std::max(260.0f, ImGui::GetContentRegionAvail().x * 0.32f));
        if (ImGui::BeginChild("AssetBrowserLeftDrawer", ImVec2(drawerWidth, 0.0f), true)) {
            ImGui::TextUnformatted("Browser");
            renderAssetBrowserDrawer(browser);
        }
        ImGui::EndChild();
        ImGui::SameLine();
    }

    if (ImGui::BeginChild("AssetLibraryMainContent", ImVec2(0.0f, 0.0f), false)) {
        if (browserAvailable) {
            if (renderAssetBrowserRowsAndPreview(browser) && useSelectedInLevelBuilder != nullptr) {
                *useSelectedInLevelBuilder = true;
            }
            ImGui::Separator();
        }

        if (ImGui::BeginTabBar("AssetLibraryTabs")) {
            if (ImGui::BeginTabItem("Wizard")) {
                ImGui::Text("Step: %s", wizard.current_step.c_str());
                ImGui::Text("Status: %s", wizard.status.c_str());
                for (const auto& step : wizard.steps) {
                    ImGui::BulletText("%s: %s (%zu)", step.label.c_str(), step.state.c_str(), step.count);
                }
                ImGui::Separator();
                for (const auto& action : wizard.actions) {
                    if (!action.enabled) {
                        ImGui::BeginDisabled();
                    }
                    ImGui::Button(action.id.c_str());
                    if (!action.enabled) {
                        ImGui::EndDisabled();
                    }
                    if (!action.disabled_reason.empty()) {
                        ImGui::SameLine();
                        ImGui::TextDisabled("%s", action.disabled_reason.c_str());
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Assets")) {
                renderJsonRows("AssetActionRows", snapshot.asset_action_rows,
                               {"path", "kind", "status", "runtime_ready", "issue_count"});
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Imports")) {
                renderJsonRows("ImportReviewRows", snapshot.import_review_rows,
                               {"asset_id", "status", "kind", "license", "reason"});
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Categories")) {
                for (const auto& [category, count] : snapshot.category_counts) {
                    ImGui::BulletText("%s: %zu", category.c_str(), count);
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();

    ImGui::End();
}
#endif

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

AssetLibraryPanel::AssetBrowserRenderSnapshot buildAssetBrowserRenderSnapshot(const AssetLibraryModelSnapshot& model) {
    AssetLibraryPanel::AssetBrowserRenderSnapshot snapshot;
    const auto& browser = model.asset_index_browser;
    if (!browser.is_object()) {
        return snapshot;
    }
    snapshot.available = browser.value("total_count", 0u) > 0 || browser.value("visible_count", 0u) > 0;
    snapshot.layout = model.asset_browser_scope.value("browser_layout", snapshot.layout);
    snapshot.query = browser.value("query", "");
    snapshot.total_count = browser.value("total_count", 0u);
    snapshot.visible_count = browser.value("visible_count", 0u);
    snapshot.filters = browser.value("filters", nlohmann::json::object());
    snapshot.page = browser.value("page", nlohmann::json::object());
    snapshot.folder_tree = browser.value("folder_tree", nlohmann::json::array());
    snapshot.category_tree = browser.value("category_tree", nlohmann::json::array());
    snapshot.pack_tree = browser.value("pack_tree", nlohmann::json::array());
    snapshot.visible_rows = browser.value("visible_rows", nlohmann::json::array());
    snapshot.selected_record = browser.value("selected_record", nlohmann::json::object());
    snapshot.preview_drawer_open = snapshot.selected_record.is_object() && !snapshot.selected_record.empty();
    snapshot.left_drawer_visible = snapshot.available && snapshot.layout == "left_collapsible_folder_tree";
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

std::optional<std::filesystem::path>
chooseNativeImportSource(const AssetLibraryPanel::ImportSourcePickerRequest& request) {
#ifdef _WIN32
    return chooseWindowsImportSource(request);
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
    return {true, true, "native_import_source_picker_available", "Native Windows import source picker is available."};
#else
    return {false, true, "native_import_source_picker_unsupported",
            "Native import source picker is not implemented on this platform; use path entry import instead."};
#endif
}

void AssetLibraryPanel::setImportSourcePicker(ImportSourcePicker picker) {
    import_source_picker_ = std::move(picker);
}

void AssetLibraryPanel::setAssetBrowserSelectionCallback(AssetBrowserSelectionCallback callback) {
    asset_browser_selection_callback_ = std::move(callback);
}

void AssetLibraryPanel::render() {
    refreshRenderSnapshotsFromModel();
    if (!visible_) {
        return;
    }
    has_rendered_frame_ = true;
#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() != nullptr) {
        bool useSelectedInLevelBuilder = false;
        renderAssetLibraryWindow(last_render_snapshot_, last_import_wizard_snapshot_, &useSelectedInLevelBuilder);
        if (useSelectedInLevelBuilder) {
            (void)dispatchSelectedAssetToLevelBuilder();
        }
    }
#endif
}

nlohmann::json AssetLibraryPanel::requestImportSource(const std::filesystem::path& source,
                                                      const std::filesystem::path& library_root, std::string session_id,
                                                      std::string license_note,
                                                      std::vector<std::string> external_extractor_command) {
    auto result = model_.requestImportSource(source, library_root, std::move(session_id), std::move(license_note),
                                             std::move(external_extractor_command));
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

nlohmann::json AssetLibraryPanel::attachSelectedPromotedAssetsToProject(std::vector<std::string> paths,
                                                                        const std::filesystem::path& project_root) {
    auto result = model_.attachPromotedAssetsToProject(std::move(paths), project_root);
    refreshRenderSnapshotsFromModel();
    return result;
}

bool AssetLibraryPanel::loadGameTemplateManifest(const std::filesystem::path& manifest_path,
                                                 std::string* error_message) {
    const bool loaded = model_.loadGameTemplateManifestFromFile(manifest_path, error_message);
    refreshRenderSnapshotsFromModel();
    return loaded;
}

bool AssetLibraryPanel::dispatchSelectedAssetToLevelBuilder() {
    refreshRenderSnapshotsFromModel();
    const auto& selected = last_asset_browser_snapshot_.selected_record;
    if (!selected.is_object() || selected.empty() || !asset_browser_selection_callback_) {
        return false;
    }
    return asset_browser_selection_callback_(selected);
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
    last_asset_browser_snapshot_ = buildAssetBrowserRenderSnapshot(last_render_snapshot_);
}

} // namespace urpg::editor
