#include "engine/core/export/runtime_bundle_loader.h"

#include "engine/core/export/export_bundle_contract.h"

#include <filesystem>
#include <system_error>

namespace urpg::exporting {

RuntimeBundleLoadResult LoadRuntimeBundle(const std::filesystem::path& bundle_path, urpg::tools::ExportTarget target) {
    RuntimeBundleLoadResult result;
    const auto validation = bundle_contract::validateBundleFile(bundle_path, target);
    result.loaded = validation.valid;
    result.manifest = validation.manifest;
    result.errors = validation.errors;
    return result;
}

bool PublishRuntimeBundleAtomic(const std::filesystem::path& source_bundle,
                                const std::filesystem::path& destination_bundle, std::string* error_message) {
    if (error_message) {
        error_message->clear();
    }
    std::error_code ec;
    std::filesystem::create_directories(destination_bundle.parent_path(), ec);
    if (ec) {
        if (error_message)
            *error_message = ec.message();
        return false;
    }

    std::filesystem::path temp_path = destination_bundle;
    temp_path += ".tmp";
    std::filesystem::copy_file(source_bundle, temp_path, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        if (error_message)
            *error_message = ec.message();
        return false;
    }

    std::filesystem::rename(temp_path, destination_bundle, ec);
    if (ec) {
        std::filesystem::remove(temp_path);
        if (error_message)
            *error_message = ec.message();
        return false;
    }
    return true;
}

} // namespace urpg::exporting
