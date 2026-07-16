#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::assets {

struct AssetAudioQualityMetadata {
    bool voice = false;
    std::string locale;
    std::string take_id;
    std::string rights_id;
    std::string caption_id;
    std::string muted_alternative_asset_id;
};

struct AssetAudioQualityInput {
    std::vector<int16_t> interleaved_samples;
    std::uint32_t channels = 1;
    std::uint32_t sample_rate = 0;
    std::int64_t loop_start_frame = -1;
    std::int64_t loop_end_frame = -1;
    AssetAudioQualityMetadata metadata;
};

struct AssetAudioQualityDiagnostic {
    std::string code;
    std::string severity;
    std::string message;
    std::string suggested_fix;
};

struct AssetAudioQualityReport {
    bool valid = false;
    double peak_dbfs = -120.0;
    double rms_dbfs = -120.0;
    double loop_max_normalized_delta = 0.0;
    std::vector<float> waveform_peaks;
    std::vector<AssetAudioQualityDiagnostic> diagnostics;
};

AssetAudioQualityReport analyzeAssetAudioQuality(const AssetAudioQualityInput& input);

} // namespace urpg::assets
