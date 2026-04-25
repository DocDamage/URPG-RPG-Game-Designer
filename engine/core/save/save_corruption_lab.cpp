#include "engine/core/save/save_corruption_lab.h"

#include <fstream>
#include <iterator>

namespace urpg::save {

namespace {

std::string readAll(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void writeAll(const std::filesystem::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
}

std::string tierLabel(urpg::SaveRecoveryTier tier) {
    switch (tier) {
    case urpg::SaveRecoveryTier::None:
        return "none";
    case urpg::SaveRecoveryTier::Level1Autosave:
        return "level1_autosave";
    case urpg::SaveRecoveryTier::Level2MetadataVariables:
        return "level2_metadata_variables";
    case urpg::SaveRecoveryTier::Level3SafeSkeleton:
        return "level3_safe_skeleton";
    }
    return "unknown";
}

} // namespace

SaveCorruptionResult SaveCorruptionLab::corruptFixtureCopy(const std::filesystem::path& source_fixture,
                                                           const std::filesystem::path& lab_directory,
                                                           const std::string& mode) const {
    SaveCorruptionResult result;
    if (!std::filesystem::exists(source_fixture)) {
        result.diagnostics.push_back("source_fixture_missing");
        return result;
    }

    std::filesystem::create_directories(lab_directory);
    result.corrupted_copy_path = lab_directory / source_fixture.filename();
    auto payload = readAll(source_fixture);
    if (mode == "truncate") {
        payload = payload.substr(0, payload.size() / 2);
    } else if (mode == "invalid_json") {
        payload = "{ invalid save json";
    } else if (mode == "wrong_schema") {
        payload = R"({"schema_version":"urpg.save.legacy","slot_id":1})";
    } else {
        result.diagnostics.push_back("unknown_corruption_mode:" + mode);
        return result;
    }

    writeAll(result.corrupted_copy_path, payload);
    result.success = true;
    return result;
}

nlohmann::json SaveCorruptionLab::simulateRecovery(const urpg::SaveRecoveryRequest& request) const {
    const auto recovery = urpg::SaveRecoveryManager::Recover(request);
    return {
        {"ok", recovery.ok},
        {"tier", tierLabel(recovery.tier)},
        {"variables_reset", recovery.variables_reset},
        {"error", recovery.error},
    };
}

} // namespace urpg::save
