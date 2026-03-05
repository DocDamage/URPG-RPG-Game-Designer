#include "engine/core/save/save_journal.h"

#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace urpg {

namespace {

void SetError(std::string* error, const std::string& value) {
    if (error) {
        *error = value;
    }
}

} // namespace

bool SaveJournal::WriteAndSync(const std::filesystem::path& path, std::string_view payload, std::string* error) {
#ifdef _WIN32
    FILE* file = nullptr;
    if (fopen_s(&file, path.string().c_str(), "wb") != 0 || file == nullptr) {
        SetError(error, "failed_to_open_temp_file");
        return false;
    }
#else
    FILE* file = std::fopen(path.string().c_str(), "wb");
    if (file == nullptr) {
        SetError(error, "failed_to_open_temp_file");
        return false;
    }
#endif

    const size_t written = std::fwrite(payload.data(), 1, payload.size(), file);
    if (written != payload.size()) {
        std::fclose(file);
        SetError(error, "failed_to_write_temp_file");
        return false;
    }

    if (std::fflush(file) != 0) {
        std::fclose(file);
        SetError(error, "failed_to_flush_temp_file");
        return false;
    }

#ifdef _WIN32
    if (_commit(_fileno(file)) != 0) {
        std::fclose(file);
        SetError(error, "failed_to_commit_temp_file");
        return false;
    }
#else
    if (fsync(fileno(file)) != 0) {
        std::fclose(file);
        SetError(error, "failed_to_commit_temp_file");
        return false;
    }
#endif

    std::fclose(file);
    return true;
}

bool SaveJournal::WriteAtomically(const std::filesystem::path& final_path, std::string_view payload, std::string* error) {
    std::error_code ec;

    const auto parent = final_path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            SetError(error, "failed_to_create_directories");
            return false;
        }
    }

    const std::filesystem::path temp_path = final_path.string() + ".tmp";
    const std::filesystem::path backup_path = final_path.string() + ".backup";

    if (!WriteAndSync(temp_path, payload, error)) {
        std::filesystem::remove(temp_path, ec);
        return false;
    }

    if (std::filesystem::exists(final_path)) {
        std::filesystem::copy_file(final_path, backup_path, std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) {
            SetError(error, "failed_to_write_backup");
            std::filesystem::remove(temp_path, ec);
            return false;
        }
        std::filesystem::remove(final_path, ec);
    }

    std::filesystem::rename(temp_path, final_path, ec);
    if (ec) {
        SetError(error, "failed_to_rename_temp_file");
        std::filesystem::remove(temp_path, ec);
        return false;
    }

    return true;
}

} // namespace urpg
