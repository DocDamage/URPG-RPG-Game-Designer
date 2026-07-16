#include "engine/core/assets/asset_audio_quality.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace urpg::assets {
namespace {

void add(AssetAudioQualityReport& report, const char* code, const char* severity, const char* message,
         const char* suggestedFix) {
    report.diagnostics.push_back({code, severity, message, suggestedFix});
}

} // namespace

AssetAudioQualityReport analyzeAssetAudioQuality(const AssetAudioQualityInput& input) {
    AssetAudioQualityReport report;
    if (input.channels == 0 || input.sample_rate == 0 || input.interleaved_samples.empty() ||
        input.interleaved_samples.size() % input.channels != 0) {
        add(report, "audio_payload_invalid", "error", "Audio samples, channel count, or sample rate are invalid.",
            "Decode the source into a supported PCM stream and retry analysis.");
        return report;
    }
    report.valid = true;
    constexpr std::size_t waveformBuckets = 128;
    report.waveform_peaks.assign(waveformBuckets, 0.0F);
    std::int32_t peak = 0;
    long double squareSum = 0.0L;
    for (std::size_t index = 0; index < input.interleaved_samples.size(); ++index) {
        const auto amplitude = std::abs(static_cast<std::int32_t>(input.interleaved_samples[index]));
        peak = std::max(peak, amplitude);
        squareSum += static_cast<long double>(amplitude) * amplitude;
        const auto bucket = std::min(waveformBuckets - 1, index * waveformBuckets / input.interleaved_samples.size());
        report.waveform_peaks[bucket] = std::max(report.waveform_peaks[bucket], static_cast<float>(amplitude) / 32768.0F);
    }
    const auto rms = std::sqrt(static_cast<double>(squareSum / input.interleaved_samples.size())) / 32768.0;
    report.peak_dbfs = peak == 0 ? -120.0 : 20.0 * std::log10(static_cast<double>(peak) / 32768.0);
    report.rms_dbfs = rms <= std::numeric_limits<double>::epsilon() ? -120.0 : 20.0 * std::log10(rms);
    if (peak >= 32760) add(report, "audio_clipping_detected", "error", "The audio reaches digital full scale and may clip.",
                           "Reduce gain or replace the clipped source take.");
    if (report.rms_dbfs < -60.0) add(report, "audio_effective_silence", "warning", "The audio is effectively silent.",
                                     "Confirm the intended take or raise source gain before revision processing.");
    if (report.rms_dbfs > -6.0) add(report, "audio_loudness_high", "warning", "Average signal level is unusually high.",
                                   "Review loudness against the project mix target and reduce gain if needed.");
    if ((input.loop_start_frame < 0) != (input.loop_end_frame < 0)) {
        add(report, "audio_loop_range_incomplete", "error", "Only one loop boundary is set.",
            "Set both loop boundaries or disable looping.");
    } else if (input.loop_start_frame >= 0) {
        const auto frameCount = static_cast<std::int64_t>(input.interleaved_samples.size() / input.channels);
        if (input.loop_start_frame >= input.loop_end_frame || input.loop_end_frame > frameCount) {
            add(report, "audio_loop_range_invalid", "error", "Loop boundaries are outside the decoded audio range.",
                "Move loop boundaries inside the take with the start before the end.");
        } else {
            for (std::uint32_t channel = 0; channel < input.channels; ++channel) {
                const auto start = input.interleaved_samples[static_cast<std::size_t>(input.loop_start_frame) * input.channels + channel];
                const auto end = input.interleaved_samples[static_cast<std::size_t>(input.loop_end_frame - 1) * input.channels + channel];
                report.loop_max_normalized_delta = std::max(report.loop_max_normalized_delta,
                    static_cast<double>(std::abs(static_cast<std::int32_t>(start) - static_cast<std::int32_t>(end))) / 65535.0);
            }
            if (report.loop_max_normalized_delta > 0.10)
                add(report, "audio_loop_seam_risky", "warning", "The loop boundary has a large sample discontinuity.",
                    "Move the loop points near a zero crossing or add a short crossfade.");
        }
    }
    if (input.metadata.rights_id.empty()) add(report, "audio_rights_missing", "error", "Audio rights metadata is missing.",
                                               "Attach a reviewed license or private-project rights classification.");
    if (input.metadata.voice) {
        if (input.metadata.locale.empty()) add(report, "audio_voice_locale_missing", "error", "Voice locale is missing.",
                                                "Assign the locale for this voice take.");
        if (input.metadata.take_id.empty()) add(report, "audio_voice_take_missing", "error", "Voice take ID is missing.",
                                                "Assign a stable take ID.");
        if (input.metadata.caption_id.empty()) add(report, "audio_caption_missing", "error", "Voice audio has no caption reference.",
                                                   "Assign a localized caption ID before packaging.");
        if (input.metadata.muted_alternative_asset_id.empty())
            add(report, "audio_muted_alternative_missing", "warning", "Voice audio has no muted-playback alternative.",
                "Assign a caption-only or muted alternative for audio-disabled play.");
    }
    return report;
}

} // namespace urpg::assets
