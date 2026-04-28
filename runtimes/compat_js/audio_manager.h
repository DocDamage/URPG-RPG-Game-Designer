#pragma once

// AudioManager - MZ Audio Middleware Compatibility Surface
// Phase 2 - Compat Layer
//
// Per Section 4 - AudioManager semantics:
// AudioManager provides buses (BGM/BGS/ME/SE), group volumes, ducking rules, and crossfade contracts.
// Crossfades are deterministic (time-based in frames in deterministic mode).

#include "engine/core/audio/audio_core.h"
#include "engine/runtimes/bridge/value.h"
#include "quickjs_runtime.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {

// Forward declarations
class AudioManagerImpl;

// Audio bus types matching MZ
enum class AudioBus : uint8_t {
    BGM = 0, // Background Music
    BGS = 1, // Background Sound
    ME = 2,  // Music Effect
    SE = 3   // Sound Effect
};

// Audio channel state
enum class AudioState : uint8_t { STOPPED = 0, PLAYING = 1, PAUSED = 2, FADING_OUT = 3, FADING_IN = 4 };

// Audio info structure (MZ format)
struct AudioInfo {
    std::string name;
    double volume = 90.0;
    double pitch = 100.0;
    int32_t pos = 0;
};

// Audio channel for playback
class AudioChannel {
  public:
    AudioChannel(const std::string& name, AudioBus bus);
    ~AudioChannel();

    // Playback control
    void play(const std::string& filename, double volume, double pitch, int32_t pos = 0);
    void stop();
    void pause();
    void resume();

    // Volume control
    void setVolume(double volume);
    double getVolume() const;

    // Pitch control
    void setPitch(double pitch);
    double getPitch() const;

    // Position
    int32_t getPosition() const;
    void setPosition(int32_t pos);

    // State queries
    bool isPlaying() const;
    bool isPaused() const;
    AudioState getState() const;

    // Bus type
    AudioBus getBus() const { return bus_; }
    const std::string& getName() const { return name_; }
    const std::string& getFilename() const { return filename_; }

    // Update (called each frame)
    void update();

    // Channel ID
    uint32_t id = 0;

  private:
    std::string name_;
    AudioBus bus_;
    AudioState state_ = AudioState::STOPPED;
    std::string filename_;
    double sourceVolume_ = 1.0;
    double volume_ = 1.0;
    double pitch_ = 1.0;
    int32_t pos_ = 0;
    bool playing_ = false;
    bool paused_ = false;
    int32_t framesUntilComplete_ = -1;

  public:
    void setSourceVolume(double volume) { sourceVolume_ = std::clamp(volume, 0.0, 1.0); }
    double getSourceVolume() const { return sourceVolume_; }
};

// AudioManager - MZ compatibility layer for audio
class AudioManager {
  public:
    AudioManager();
    ~AudioManager();

    // Non-copyable
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Singleton access for compatibility
    static AudioManager& instance();

    void bindAudioCore(urpg::audio::AudioCore* core);
    urpg::audio::AudioCore* boundAudioCore() const;

    // ========================================================================
    // Channel Management
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    uint32_t createChannel(const std::string& name, AudioBus bus);
    void destroyChannel(const std::string& name);
    void destroyChannel(uint32_t id);

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    AudioChannel* getChannel(const std::string& name);
    AudioChannel* getChannel(uint32_t id);

    // ========================================================================
    // BGM Control
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void playBgm(const std::string& filename, double volume = 90.0, double pitch = 100.0, int32_t pos = 0);

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void stopBgm();

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void pauseBgm();
    void resumeBgm();

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void crossfadeBgm(const std::string& filename, double volume = 90.0, double pitch = 100.0, int32_t duration = 60);

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void saveBgmSettings();
    void restoreBgmSettings();

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    bool isBgmPlaying() const;
    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    bool isBgmPaused() const;
    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    AudioInfo getCurrentBgm() const;

    // ========================================================================
    // BGS Control
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void playBgs(const std::string& filename, double volume = 90.0, double pitch = 100.0, int32_t pos = 0);

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void stopBgs();

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void crossfadeBgs(const std::string& filename, double volume = 90.0, double pitch = 100.0, int32_t duration = 60);

    // ========================================================================
    // ME Control
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void playMe(const std::string& filename, double volume = 90.0, double pitch = 100.0);

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void stopMe();

    // ========================================================================
    // SE Control
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void playSe(const std::string& filename, double volume = 90.0, double pitch = 100.0);

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void stopSe();

    // ========================================================================
    // Volume Control
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void setMasterVolume(double volume);
    double getMasterVolume() const;

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void setBusVolume(AudioBus bus, double volume);
    double getBusVolume(AudioBus bus) const;

    // ========================================================================
    // Ducking
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void duckBgm(double volume = 50.0, int32_t duration = 30);
    void unduckBgm(int32_t duration = 30);
    bool isBgmDucked() const;

    // ========================================================================
    // Update
    // ========================================================================

    // Status: FULL - Backed by deterministic compat state and bound AudioCore integration where runtime playback is available.
    void update();

    // ========================================================================
    // Compat Status
    // ========================================================================

    // Register AudioManager API with QuickJS context
    static void registerAPI(QuickJSContext& ctx);

    // Get compat status for a method
    static CompatStatus getMethodStatus(const std::string& methodName);
    static std::string getMethodDeviation(const std::string& methodName);

  private:
    std::unique_ptr<AudioManagerImpl> impl_;

    // API status registry
    static std::unordered_map<std::string, CompatStatus> methodStatus_;
    static std::unordered_map<std::string, std::string> methodDeviations_;
};

} // namespace compat
} // namespace urpg
