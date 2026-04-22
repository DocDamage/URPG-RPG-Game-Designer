#pragma once

// AudioManager - MZ Audio Middleware Compatibility Surface
// Phase 2 - Compat Layer
//
// Per Section 4 - AudioManager semantics:
// AudioManager provides buses (BGM/BGS/ME/SE), group volumes, ducking rules, and crossfade contracts.
// Crossfades are deterministic (time-based in frames in deterministic mode).

#include "quickjs_runtime.h"
#include "engine/runtimes/bridge/value.h"
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
    BGM = 0,   // Background Music
    BGS = 1,   // Background Sound
    ME = 2,    // Music Effect
    SE = 3     // Sound Effect
};

// Audio channel state
enum class AudioState : uint8_t {
    STOPPED = 0,
    PLAYING = 1,
    PAUSED = 2,
    FADING_OUT = 3,
    FADING_IN = 4
};

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
    
    // ========================================================================
    // Channel Management
    // ========================================================================
    
    // Status: PARTIAL - Channel lifecycle is deterministic in-harness, not backed by a live audio engine
    uint32_t createChannel(const std::string& name, AudioBus bus);
    void destroyChannel(const std::string& name);
    void destroyChannel(uint32_t id);
    
    // Status: PARTIAL - Exposes harness-side channels rather than live backend mixer channels
    AudioChannel* getChannel(const std::string& name);
    AudioChannel* getChannel(uint32_t id);
    
    // ========================================================================
    // BGM Control
    // ========================================================================
    
    // Status: PARTIAL - Drives deterministic harness state rather than a live audio backend
    void playBgm(const std::string& filename, double volume = 90.0, 
                 double pitch = 100.0, int32_t pos = 0);
    
    // Status: PARTIAL - Stops deterministic harness state rather than a live audio backend
    void stopBgm();
    
    // Status: PARTIAL - Pauses/resumes deterministic harness state rather than a live audio backend
    void pauseBgm();
    void resumeBgm();
    
    // Status: PARTIAL - Deterministic frame-based crossfade in the harness, not a live mixer/backend
    void crossfadeBgm(const std::string& filename, double volume = 90.0,
                       double pitch = 100.0, int32_t duration = 60);
    
    // Status: PARTIAL - Saves deterministic harness playback metadata, not live backend state
    void saveBgmSettings();
    void restoreBgmSettings();
    
    // Status: PARTIAL - Reflects deterministic harness playback state rather than live backend state
    bool isBgmPlaying() const;
    // Status: PARTIAL - Reflects deterministic harness playback state rather than live backend state
    bool isBgmPaused() const;
    // Status: PARTIAL - Reports deterministic harness playback metadata and mix-scaled BGM state through the compat API rather than live backend state
    AudioInfo getCurrentBgm() const;
    
    // ========================================================================
    // BGS Control
    // ========================================================================
    
    // Status: PARTIAL - Drives deterministic harness state rather than a live audio backend
    void playBgs(const std::string& filename, double volume = 90.0,
                 double pitch = 100.0, int32_t pos = 0);
    
    // Status: PARTIAL - Stops deterministic harness state rather than a live audio backend
    void stopBgs();
    
    // Status: PARTIAL - Deterministic frame-based crossfade in the harness, not a live mixer/backend
    void crossfadeBgs(const std::string& filename, double volume = 90.0,
                       double pitch = 100.0, int32_t duration = 60);
    
    // ========================================================================
    // ME Control
    // ========================================================================
    
    // Status: PARTIAL - Drives deterministic harness state rather than a live audio backend
    void playMe(const std::string& filename, double volume = 90.0,
                double pitch = 100.0);
    
    // Status: PARTIAL - Stops deterministic harness state rather than a live audio backend
    void stopMe();
    
    // ========================================================================
    // SE Control
    // ========================================================================
    
    // Status: PARTIAL - Drives deterministic harness state rather than a live audio backend
    void playSe(const std::string& filename, double volume = 90.0,
                double pitch = 100.0);
    
    // Status: PARTIAL - Stops deterministic harness state rather than a live audio backend
    void stopSe();
    
    // ========================================================================
    // Volume Control
    // ========================================================================
    
    // Status: PARTIAL - Applies deterministic harness mix scaling rather than a live mixer/backend
    void setMasterVolume(double volume);
    double getMasterVolume() const;
    
    // Status: PARTIAL - Applies deterministic harness mix scaling rather than a live mixer/backend
    void setBusVolume(AudioBus bus, double volume);
    double getBusVolume(AudioBus bus) const;
    
    // ========================================================================
    // Ducking
    // ========================================================================
    
    // Status: PARTIAL - Deterministic duck/unduck and current BGM state remain observable through the compat API, but this still does not drive a live mixer/backend
    void duckBgm(double volume = 50.0, int32_t duration = 30);
    void unduckBgm(int32_t duration = 30);
    bool isBgmDucked() const;
    
    // ========================================================================
    // Update
    // ========================================================================
    
    // Status: PARTIAL - Advances deterministic harness playback and cleanup, not a live audio engine
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
