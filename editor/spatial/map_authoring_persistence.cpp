#include "editor/spatial/map_authoring_persistence.h"

#include <fstream>
#include <set>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace urpg::editor {
namespace {

bool replaceFile(const std::filesystem::path& source, const std::filesystem::path& destination, std::error_code& error) {
#ifdef _WIN32
    if (MoveFileExW(source.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        return true;
    }
    error = std::error_code(static_cast<int>(GetLastError()), std::system_category());
    return false;
#else
    std::filesystem::rename(source, destination, error);
    return !error;
#endif
}

void removeQuietly(const std::filesystem::path& path) {
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

} // namespace

MapAuthoringPersistenceResult publishMapAuthoringDocuments(const std::vector<MapAuthoringDocumentWrite>& documents) {
    if (documents.empty()) {
        return {false, "map_save_no_documents", "No map documents were supplied for publication."};
    }

    std::set<std::filesystem::path> targets;
    for (const auto& document : documents) {
        if (document.target.empty() || !targets.insert(document.target.lexically_normal()).second) {
            return {false, "map_save_invalid_targets", "Map save targets must be non-empty and unique."};
        }
    }

    std::vector<std::filesystem::path> temporary;
    std::vector<std::filesystem::path> backup;
    temporary.reserve(documents.size());
    backup.reserve(documents.size());
    for (size_t index = 0; index < documents.size(); ++index) {
        const auto& document = documents[index];
        std::error_code error;
        std::filesystem::create_directories(document.target.parent_path(), error);
        if (error) {
            for (const auto& path : temporary) removeQuietly(path);
            return {false, "map_save_directory_create_failed", "Could not create the map document directory: " + error.message()};
        }
        const auto suffix = ".urpg-map-save-" + std::to_string(index);
        const auto temp = document.target.parent_path() / ("." + document.target.filename().string() + suffix + ".tmp");
        const auto saved = document.target.parent_path() / ("." + document.target.filename().string() + suffix + ".bak");
        removeQuietly(temp);
        removeQuietly(saved);
        std::ofstream output(temp, std::ios::binary | std::ios::trunc);
        output << document.contents;
        if (!output) {
            output.close();
            removeQuietly(temp);
            for (const auto& path : temporary) removeQuietly(path);
            return {false, "map_save_temporary_write_failed", "Could not write a temporary map document."};
        }
        output.close();
        temporary.push_back(temp);
        backup.push_back(saved);
    }

    std::vector<bool> hadOriginal(documents.size(), false);
    for (size_t index = 0; index < documents.size(); ++index) {
        std::error_code error;
        hadOriginal[index] = std::filesystem::exists(documents[index].target, error) && !error;
        if (hadOriginal[index]) {
            std::filesystem::copy_file(documents[index].target, backup[index],
                                       std::filesystem::copy_options::overwrite_existing, error);
            if (error) {
                for (const auto& path : temporary) removeQuietly(path);
                for (const auto& path : backup) removeQuietly(path);
                return {false, "map_save_backup_failed", "Could not back up the previous map document: " + error.message()};
            }
        }
    }

    size_t published = 0;
    for (; published < documents.size(); ++published) {
        std::error_code error;
        if (replaceFile(temporary[published], documents[published].target, error)) {
            continue;
        }
        for (size_t rollback = 0; rollback < published; ++rollback) {
            std::error_code rollbackError;
            if (hadOriginal[rollback]) {
                (void)replaceFile(backup[rollback], documents[rollback].target, rollbackError);
            } else {
                removeQuietly(documents[rollback].target);
            }
        }
        for (size_t cleanup = published; cleanup < temporary.size(); ++cleanup) removeQuietly(temporary[cleanup]);
        for (const auto& path : backup) removeQuietly(path);
        return {false, "map_save_publish_failed", "Map save failed and prior documents were restored: " + error.message()};
    }
    for (const auto& path : backup) removeQuietly(path);
    return {true, "map_documents_published", "All map documents were published together."};
}

} // namespace urpg::editor
