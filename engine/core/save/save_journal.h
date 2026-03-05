#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace urpg {

class SaveJournal {
public:
    static bool WriteAtomically(const std::filesystem::path& final_path, std::string_view payload, std::string* error = nullptr);

private:
    static bool WriteAndSync(const std::filesystem::path& path, std::string_view payload, std::string* error);
};

} // namespace urpg
