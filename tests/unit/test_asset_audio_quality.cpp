#include "engine/core/assets/asset_audio_quality.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {
bool hasCode(const urpg::assets::AssetAudioQualityReport& report, const std::string& code) {
    return std::any_of(report.diagnostics.begin(), report.diagnostics.end(),
                       [&](const auto& diagnostic) { return diagnostic.code == code; });
}
}

TEST_CASE("Asset audio QA golden corpus diagnoses clipping silence loops and voice custody", "[assets][audio_quality]") {
    using namespace urpg::assets;
    AssetAudioQualityInput clipped{{32767, -32768, 32767, -32768}, 1, 48000, -1, -1,
                                   {false, {}, {}, "rights.cc0", {}, {}}};
    const auto clipping = analyzeAssetAudioQuality(clipped);
    REQUIRE(clipping.valid);
    REQUIRE(hasCode(clipping, "audio_clipping_detected"));
    REQUIRE(hasCode(clipping, "audio_loudness_high"));
    REQUIRE(clipping.waveform_peaks.size() == 128);

    AssetAudioQualityInput silent{{0, 0, 0, 0}, 1, 48000, -1, -1,
                                  {false, {}, {}, "rights.private", {}, {}}};
    const auto silence = analyzeAssetAudioQuality(silent);
    REQUIRE(hasCode(silence, "audio_effective_silence"));

    AssetAudioQualityInput badLoop{{30000, 1000, -30000, -1000}, 1, 48000, 0, 3,
                                   {false, {}, {}, "rights.private", {}, {}}};
    const auto loop = analyzeAssetAudioQuality(badLoop);
    REQUIRE(hasCode(loop, "audio_loop_seam_risky"));
    REQUIRE(loop.loop_max_normalized_delta > 0.10);

    AssetAudioQualityInput voice{{1000, -1000, 1000, -1000}, 1, 48000, -1, -1,
                                 {true, "en-US", "take-01", "rights.voice", {}, {}}};
    const auto missingVoiceMetadata = analyzeAssetAudioQuality(voice);
    REQUIRE(hasCode(missingVoiceMetadata, "audio_caption_missing"));
    REQUIRE(hasCode(missingVoiceMetadata, "audio_muted_alternative_missing"));
    REQUIRE_FALSE(hasCode(missingVoiceMetadata, "audio_voice_locale_missing"));
    REQUIRE_FALSE(hasCode(missingVoiceMetadata, "audio_voice_take_missing"));

    voice.metadata.caption_id = "dialogue.guide.caption";
    voice.metadata.muted_alternative_asset_id = "voice.guide.muted";
    const auto complete = analyzeAssetAudioQuality(voice);
    REQUIRE_FALSE(hasCode(complete, "audio_caption_missing"));
    REQUIRE_FALSE(hasCode(complete, "audio_muted_alternative_missing"));
    REQUIRE_FALSE(hasCode(complete, "audio_rights_missing"));
}

TEST_CASE("Asset audio QA refuses malformed decoded payloads", "[assets][audio_quality]") {
    const auto report = urpg::assets::analyzeAssetAudioQuality({{1, 2, 3}, 2, 48000, -1, -1, {}});
    REQUIRE_FALSE(report.valid);
    REQUIRE(hasCode(report, "audio_payload_invalid"));
}
