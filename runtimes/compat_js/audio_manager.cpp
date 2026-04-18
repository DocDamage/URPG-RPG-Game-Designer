// AudioManager - MZ Audio Middleware Compatibility Surface - Implementation
// Phase 2 - Compat Layer

#include "audio_manager.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace urpg {
namespace compat {

// Static member definitions
std::unordered_map<std::string, CompatStatus> AudioManager::methodStatus_;
std::unordered_map<std::string, std::string> AudioManager::methodDeviations_;

// ============================================================================
// AudioChannel Implementation
// ============================================================================

AudioChannel::AudioChannel(const std::string& name, AudioBus bus)
    : name_(name), bus_(bus) {
}

AudioChannel::~AudioChannel() = default;

void AudioChannel::play(const std::string& filename, double volume, double pitch, int32_t pos) {
    filename_ = filename;
    volume_ = std::clamp(volume / 100.0, 0.0, 1.0);
    pitch_ = std::clamp(pitch / 100.0, 0.5, 2.0);
    pos_ = std::max(0, pos);
    state_ = AudioState::PLAYING;
    playing_ = true;
    paused_ = false;
    elapsedFrames_ = 0;
}

void AudioChannel::stop() {
    state_ = AudioState::STOPPED;
    playing_ = false;
    paused_ = false;
    pos_ = 0;
}

void AudioChannel::pause() {
    if (playing_ && !paused_) {
        paused_ = true;
        state_ = AudioState::PAUSED;
    }
}

void AudioChannel::resume() {
    if (playing_ && paused_) {
        paused_ = false;
        state_ = AudioState::PLAYING;
    }
}

void AudioChannel::setVolume(double volume) {
    volume_ = std::clamp(volume, 0.0, 1.0);
}

double AudioChannel::getVolume() const {
    return volume_;
}

void AudioChannel::setPitch(double pitch) {
    pitch_ = std::clamp(pitch, 0.5, 2.0);
}

double AudioChannel::getPitch() const {
    return pitch_;
}

int32_t AudioChannel::getPosition() const {
    return pos_;
}

void AudioChannel::setPosition(int32_t pos) {
    pos_ = std::max(0, pos);
}

bool AudioChannel::isPlaying() const {
    return playing_ && !paused_;
}

bool AudioChannel::isPaused() const {
    return paused_;
}

AudioState AudioChannel::getState() const {
    return state_;
}

void AudioChannel::setDurationFrames(int32_t frames) {
    durationFrames_ = std::max(0, frames);
}

int32_t AudioChannel::getDurationFrames() const {
    return durationFrames_;
}

int32_t AudioChannel::getElapsedFrames() const {
    return elapsedFrames_;
}

void AudioChannel::update() {
    if (playing_ && !paused_) {
        pos_++;
        if (durationFrames_ > 0) {
            elapsedFrames_++;
            if (elapsedFrames_ >= durationFrames_) {
                stop();
            }
        }
    }
}

// ============================================================================
// AudioManagerImpl
// ============================================================================

struct PendingCrossfade {
    bool active = false;
    std::string filename;
    double targetVolumePercent = 90.0;
    double targetPitchPercent = 100.0;
    int32_t targetPos = 0;
    int32_t durationFrames = 0;
    int32_t elapsedFrames = 0;
    double sourceVolume = 1.0;
    bool switchedTrack = false;
};

struct PendingDuck {
    bool active = false;
    double targetVolume = 1.0;
    double sourceVolume = 1.0;
    int32_t durationFrames = 0;
    int32_t elapsedFrames = 0;
};

class AudioManagerImpl {
public:
    std::vector<std::unique_ptr<AudioChannel>> channels_;
    std::unordered_map<std::string, uint32_t> channelIndex_;
    std::unordered_map<AudioBus, double> busVolumes_;
    double masterVolume_ = 1.0;
    uint32_t nextChannelId_ = 1;
    
    // BGM state
    AudioChannel* bgmChannel_ = nullptr;
    AudioInfo savedBgm_;
    bool bgmDucked_ = false;
    double bgmDuckVolume_ = 1.0;
    PendingCrossfade bgmCrossfade_;
    PendingDuck bgmDuck_;

    // BGS state
    AudioChannel* bgsChannel_ = nullptr;
    PendingCrossfade bgsCrossfade_;
    
    // ME state
    AudioChannel* meChannel_ = nullptr;
    
    // SE channels
    std::vector<AudioChannel*> seChannels_;
    
    AudioManagerImpl() {
        busVolumes_[AudioBus::BGM] = 1.0;
        busVolumes_[AudioBus::BGS] = 1.0;
        busVolumes_[AudioBus::ME] = 1.0;
        busVolumes_[AudioBus::SE] = 1.0;
    }
};

// ============================================================================
// AudioManager Implementation
// ============================================================================

AudioManager::AudioManager()
    : impl_(std::make_unique<AudioManagerImpl>())
{
    // Initialize method status registry
    if (methodStatus_.empty()) {
        const auto setStatus = [](const std::string& method,
                                  CompatStatus status,
                                  const std::string& deviation = "") {
            methodStatus_[method] = status;
            if (deviation.empty()) {
                methodDeviations_.erase(method);
            } else {
                methodDeviations_[method] = deviation;
            }
        };

        // BGM
        setStatus("playBgm", CompatStatus::FULL);
        setStatus("stopBgm", CompatStatus::FULL);
        setStatus("pauseBgm", CompatStatus::FULL);
        setStatus("resumeBgm", CompatStatus::FULL);
        setStatus("crossfadeBgm", CompatStatus::FULL);
        setStatus("saveBgmSettings", CompatStatus::PARTIAL,
                  "State snapshot round-trips filename/volume/pitch, but playback position does not advance yet.");
        setStatus("restoreBgmSettings", CompatStatus::PARTIAL,
                  "State restore round-trips filename/volume/pitch, but playback position does not advance yet.");
        setStatus("isBgmPlaying", CompatStatus::FULL);
        setStatus("isBgmPaused", CompatStatus::FULL);
        setStatus("getCurrentBgm", CompatStatus::PARTIAL,
                  "Reports channel metadata, but playback position remains static because timed progression is TODO.");
        
        // BGS
        setStatus("playBgs", CompatStatus::FULL);
        setStatus("stopBgs", CompatStatus::FULL);
        setStatus("crossfadeBgs", CompatStatus::FULL);
        
        // ME
        setStatus("playMe", CompatStatus::FULL);
        setStatus("stopMe", CompatStatus::FULL);
        
        // SE
        setStatus("playSe", CompatStatus::FULL);
        setStatus("stopSe", CompatStatus::FULL);
        
        // Volume
        setStatus("setMasterVolume", CompatStatus::FULL);
        setStatus("getMasterVolume", CompatStatus::FULL);
        setStatus("setBusVolume", CompatStatus::FULL);
        setStatus("getBusVolume", CompatStatus::FULL);
        
        // Ducking
        setStatus("duckBgm", CompatStatus::PARTIAL,
                  "Ducking applies immediately; smooth duration-based ducking is still TODO.");
        setStatus("unduckBgm", CompatStatus::PARTIAL,
                  "Unducking applies immediately; smooth duration-based restore is still TODO.");
        setStatus("isBgmDucked", CompatStatus::FULL);
        
        // Channels
        setStatus("createChannel", CompatStatus::FULL);
        setStatus("destroyChannel", CompatStatus::FULL);
        setStatus("getChannel", CompatStatus::FULL);
    }
}

AudioManager::~AudioManager() = default;

AudioManager& AudioManager::instance() {
    static AudioManager instance;
    return instance;
}

// ============================================================================
// Channel Management
// ============================================================================

uint32_t AudioManager::createChannel(const std::string& name, AudioBus bus) {
    auto channel = std::make_unique<AudioChannel>(name, bus);
    uint32_t id = impl_->nextChannelId_++;
    channel->id = id;
    
    impl_->channelIndex_[name] = id;
    impl_->channels_.push_back(std::move(channel));
    
    return id;
}

void AudioManager::destroyChannel(const std::string& name) {
    auto it = impl_->channelIndex_.find(name);
    if (it != impl_->channelIndex_.end()) {
        destroyChannel(it->second);
    }
}

void AudioManager::destroyChannel(uint32_t id) {
    for (auto it = impl_->channels_.begin(); it != impl_->channels_.end(); ++it) {
        if ((*it)->id == id) {
            // Remove from index
            for (auto idxIt = impl_->channelIndex_.begin(); idxIt != impl_->channelIndex_.end(); ) {
                if (idxIt->second == id) {
                    idxIt = impl_->channelIndex_.erase(idxIt);
                } else {
                    ++idxIt;
                }
            }
            impl_->channels_.erase(it);
            break;
        }
    }
}

AudioChannel* AudioManager::getChannel(const std::string& name) {
    auto it = impl_->channelIndex_.find(name);
    if (it != impl_->channelIndex_.end()) {
        return getChannel(it->second);
    }
    return nullptr;
}

AudioChannel* AudioManager::getChannel(uint32_t id) {
    for (auto& channel : impl_->channels_) {
        if (channel->id == id) {
            return channel.get();
        }
    }
    return nullptr;
}

// ============================================================================
// BGM Control
// ============================================================================

void AudioManager::playBgm(const std::string& filename, double volume, double pitch, int32_t pos) {
    impl_->bgmCrossfade_.active = false;
    if (!impl_->bgmChannel_) {
        createChannel("bgm", AudioBus::BGM);
        impl_->bgmChannel_ = getChannel("bgm");
    }
    
    if (impl_->bgmChannel_) {
        impl_->bgmChannel_->play(filename, volume, pitch, pos);
    }
}

void AudioManager::stopBgm() {
    impl_->bgmCrossfade_.active = false;
    if (impl_->bgmChannel_) {
        impl_->bgmChannel_->stop();
    }
}

void AudioManager::pauseBgm() {
    if (impl_->bgmChannel_) {
        impl_->bgmChannel_->pause();
    }
}

void AudioManager::resumeBgm() {
    if (impl_->bgmChannel_) {
        impl_->bgmChannel_->resume();
    }
}

void AudioManager::crossfadeBgm(const std::string& filename, double volume, double pitch, int32_t duration) {
    if (!impl_->bgmChannel_ || !impl_->bgmChannel_->isPlaying() || duration <= 0) {
        playBgm(filename, volume, pitch);
        return;
    }

    impl_->bgmCrossfade_.active = true;
    impl_->bgmCrossfade_.filename = filename;
    impl_->bgmCrossfade_.targetVolumePercent = std::clamp(volume, 0.0, 100.0);
    impl_->bgmCrossfade_.targetPitchPercent = std::clamp(pitch, 50.0, 200.0);
    impl_->bgmCrossfade_.targetPos = 0;
    impl_->bgmCrossfade_.durationFrames = std::max(1, duration);
    impl_->bgmCrossfade_.elapsedFrames = 0;
    impl_->bgmCrossfade_.sourceVolume = std::clamp(impl_->bgmChannel_->getVolume(), 0.0, 1.0);
    impl_->bgmCrossfade_.switchedTrack = false;
}

void AudioManager::saveBgmSettings() {
    if (impl_->bgmChannel_) {
        impl_->savedBgm_.name = impl_->bgmChannel_->getFilename();
        impl_->savedBgm_.volume = impl_->bgmChannel_->getVolume() * 100.0;
        impl_->savedBgm_.pitch = impl_->bgmChannel_->getPitch() * 100.0;
        impl_->savedBgm_.pos = impl_->bgmChannel_->getPosition();
    }
}

void AudioManager::restoreBgmSettings() {
    if (!impl_->savedBgm_.name.empty()) {
        playBgm(impl_->savedBgm_.name, impl_->savedBgm_.volume, 
                impl_->savedBgm_.pitch, impl_->savedBgm_.pos);
    }
}

bool AudioManager::isBgmPlaying() const {
    return impl_->bgmChannel_ && impl_->bgmChannel_->isPlaying();
}

bool AudioManager::isBgmPaused() const {
    return impl_->bgmChannel_ && impl_->bgmChannel_->isPaused();
}

AudioInfo AudioManager::getCurrentBgm() const {
    AudioInfo info;
    if (impl_->bgmChannel_) {
        info.name = impl_->bgmChannel_->getFilename();
        info.volume = impl_->bgmChannel_->getVolume() * 100.0;
        info.pitch = impl_->bgmChannel_->getPitch() * 100.0;
        info.pos = impl_->bgmChannel_->getPosition();
    }
    return info;
}

// ============================================================================
// BGS Control
// ============================================================================

void AudioManager::playBgs(const std::string& filename, double volume, double pitch, int32_t pos) {
    impl_->bgsCrossfade_.active = false;
    if (!impl_->bgsChannel_) {
        createChannel("bgs", AudioBus::BGS);
        impl_->bgsChannel_ = getChannel("bgs");
    }
    
    if (impl_->bgsChannel_) {
        impl_->bgsChannel_->play(filename, volume, pitch, pos);
    }
}

void AudioManager::stopBgs() {
    impl_->bgsCrossfade_.active = false;
    if (impl_->bgsChannel_) {
        impl_->bgsChannel_->stop();
    }
}

void AudioManager::crossfadeBgs(const std::string& filename, double volume, double pitch, int32_t duration) {
    if (!impl_->bgsChannel_ || !impl_->bgsChannel_->isPlaying() || duration <= 0) {
        playBgs(filename, volume, pitch);
        return;
    }

    impl_->bgsCrossfade_.active = true;
    impl_->bgsCrossfade_.filename = filename;
    impl_->bgsCrossfade_.targetVolumePercent = std::clamp(volume, 0.0, 100.0);
    impl_->bgsCrossfade_.targetPitchPercent = std::clamp(pitch, 50.0, 200.0);
    impl_->bgsCrossfade_.targetPos = 0;
    impl_->bgsCrossfade_.durationFrames = std::max(1, duration);
    impl_->bgsCrossfade_.elapsedFrames = 0;
    impl_->bgsCrossfade_.sourceVolume = std::clamp(impl_->bgsChannel_->getVolume(), 0.0, 1.0);
    impl_->bgsCrossfade_.switchedTrack = false;
}

// ============================================================================
// ME Control
// ============================================================================

void AudioManager::playMe(const std::string& filename, double volume, double pitch) {
    if (!impl_->meChannel_) {
        createChannel("me", AudioBus::ME);
        impl_->meChannel_ = getChannel("me");
    }
    
    if (impl_->meChannel_) {
        impl_->meChannel_->play(filename, volume, pitch);
    }
}

void AudioManager::stopMe() {
    if (impl_->meChannel_) {
        impl_->meChannel_->stop();
    }
}

// ============================================================================
// SE Control
// ============================================================================

void AudioManager::playSe(const std::string& filename, double volume, double pitch) {
    // Create a new SE channel for each sound effect
    uint32_t id = createChannel("se_" + std::to_string(impl_->nextChannelId_), AudioBus::SE);
    AudioChannel* channel = getChannel(id);
    if (channel) {
        channel->play(filename, volume, pitch);
        channel->setDurationFrames(60); // Default SE duration: 60 frames (~1 sec at 60fps)
        impl_->seChannels_.push_back(channel);
    }
}

void AudioManager::stopSe() {
    for (auto* channel : impl_->seChannels_) {
        if (channel) {
            channel->stop();
        }
    }
    impl_->seChannels_.clear();
}

// ============================================================================
// Volume Control
// ============================================================================

void AudioManager::setMasterVolume(double volume) {
    impl_->masterVolume_ = std::clamp(volume, 0.0, 1.0);
}

double AudioManager::getMasterVolume() const {
    return impl_->masterVolume_;
}

void AudioManager::setBusVolume(AudioBus bus, double volume) {
    impl_->busVolumes_[bus] = std::clamp(volume, 0.0, 1.0);
}

double AudioManager::getBusVolume(AudioBus bus) const {
    auto it = impl_->busVolumes_.find(bus);
    return it != impl_->busVolumes_.end() ? it->second : 1.0;
}

// ============================================================================
// Ducking
// ============================================================================

void AudioManager::duckBgm(double volume, int32_t duration) {
    impl_->bgmDucked_ = true;
    impl_->bgmDuckVolume_ = std::clamp(volume / 100.0, 0.0, 1.0);
    
    if (impl_->bgmChannel_) {
        if (duration > 0) {
            impl_->bgmDuck_.active = true;
            impl_->bgmDuck_.sourceVolume = impl_->bgmChannel_->getVolume();
            impl_->bgmDuck_.targetVolume = impl_->bgmDuckVolume_;
            impl_->bgmDuck_.durationFrames = duration;
            impl_->bgmDuck_.elapsedFrames = 0;
        } else {
            impl_->bgmChannel_->setVolume(impl_->bgmDuckVolume_);
        }
    }
}

void AudioManager::unduckBgm(int32_t duration) {
    impl_->bgmDucked_ = false;
    
    if (impl_->bgmChannel_) {
        if (duration > 0) {
            impl_->bgmDuck_.active = true;
            impl_->bgmDuck_.sourceVolume = impl_->bgmChannel_->getVolume();
            impl_->bgmDuck_.targetVolume = 1.0;
            impl_->bgmDuck_.durationFrames = duration;
            impl_->bgmDuck_.elapsedFrames = 0;
        } else {
            impl_->bgmChannel_->setVolume(1.0);
        }
    }
}

bool AudioManager::isBgmDucked() const {
    return impl_->bgmDucked_;
}

// ============================================================================
// Update
// ============================================================================

size_t AudioManager::getSeChannelCount() const {
    return impl_->seChannels_.size();
}

void AudioManager::update() {
    auto stepCrossfade = [](PendingCrossfade& state, AudioChannel* channel) {
        if (!state.active || !channel) {
            return;
        }

        const int32_t totalFrames = std::max(1, state.durationFrames);
        const int32_t fadeOutFrames = std::max(1, totalFrames / 2);
        state.elapsedFrames = std::min(totalFrames, state.elapsedFrames + 1);

        if (!state.switchedTrack) {
            const double fadeOutT = std::min(1.0, static_cast<double>(state.elapsedFrames) / fadeOutFrames);
            channel->setVolume(state.sourceVolume * (1.0 - fadeOutT));
            if (state.elapsedFrames >= fadeOutFrames) {
                channel->play(
                    state.filename,
                    state.targetVolumePercent,
                    state.targetPitchPercent,
                    state.targetPos);
                channel->setVolume(0.0);
                state.switchedTrack = true;
            }
        }

        if (state.switchedTrack) {
            const int32_t fadeInFrames = std::max(1, totalFrames - fadeOutFrames);
            const int32_t fadeInElapsed = std::max(0, state.elapsedFrames - fadeOutFrames);
            const double fadeInT = std::min(1.0, static_cast<double>(fadeInElapsed) / fadeInFrames);
            channel->setVolume((state.targetVolumePercent / 100.0) * fadeInT);
        }

        if (state.elapsedFrames >= totalFrames) {
            channel->setVolume(state.targetVolumePercent / 100.0);
            state.active = false;
        }
    };

    stepCrossfade(impl_->bgmCrossfade_, impl_->bgmChannel_);
    stepCrossfade(impl_->bgsCrossfade_, impl_->bgsChannel_);

    // Duck interpolation runs after crossfade so that ducking can override
    // crossfade volume on the same frame.
    if (impl_->bgmDuck_.active && impl_->bgmChannel_) {
        auto& d = impl_->bgmDuck_;
        d.elapsedFrames++;
        const double t = std::min(1.0, static_cast<double>(d.elapsedFrames) / std::max(1, d.durationFrames));
        const double vol = d.sourceVolume + (d.targetVolume - d.sourceVolume) * t;
        impl_->bgmChannel_->setVolume(vol);
        if (d.elapsedFrames >= d.durationFrames) {
            d.active = false;
        }
    }

    // Update all channels
    for (auto& channel : impl_->channels_) {
        if (channel) {
            channel->update();
        }
    }
    
    // Clean up finished SE channels
    for (auto it = impl_->seChannels_.begin(); it != impl_->seChannels_.end(); ) {
        if (*it && !(*it)->isPlaying()) {
            destroyChannel((*it)->id);
            it = impl_->seChannels_.erase(it);
        } else {
            ++it;
        }
    }
}

// ============================================================================
// Compat Status
// ============================================================================

CompatStatus AudioManager::getMethodStatus(const std::string& methodName) {
    auto it = methodStatus_.find(methodName);
    if (it != methodStatus_.end()) {
        return it->second;
    }
    return CompatStatus::UNSUPPORTED;
}

std::string AudioManager::getMethodDeviation(const std::string& methodName) {
    auto it = methodDeviations_.find(methodName);
    if (it != methodDeviations_.end()) {
        return it->second;
    }
    return "";
}

namespace {

int64_t valueToInt64(const urpg::Value& value, int64_t fallback = 0) {
    if (const auto* integer = std::get_if<int64_t>(&value.v)) {
        return *integer;
    }
    if (const auto* real = std::get_if<double>(&value.v)) {
        return static_cast<int64_t>(std::llround(*real));
    }
    if (const auto* flag = std::get_if<bool>(&value.v)) {
        return *flag ? 1 : 0;
    }
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        try {
            size_t consumed = 0;
            const int64_t parsed = std::stoll(*text, &consumed, 10);
            if (consumed == text->size()) {
                return parsed;
            }
        } catch (...) {
        }
        try {
            size_t consumed = 0;
            const double parsed = std::stod(*text, &consumed);
            if (consumed == text->size()) {
                return static_cast<int64_t>(std::llround(parsed));
            }
        } catch (...) {
        }
    }
    return fallback;
}

std::string valueToString(const urpg::Value& value, const std::string& fallback = "") {
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        return *text;
    }
    return fallback;
}

} // namespace

void AudioManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"playBgm", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playBgm(
            valueToString(args[0]),
            args.size() > 1 ? static_cast<double>(valueToInt64(args[1])) : 90.0,
            args.size() > 2 ? static_cast<double>(valueToInt64(args[2])) : 100.0);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"stopBgm", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopBgm();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"pauseBgm", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().pauseBgm();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"resumeBgm", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().resumeBgm();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"isBgmPlaying", [](const std::vector<Value>&) -> Value {
        return Value::Int(AudioManager::instance().isBgmPlaying() ? 1 : 0);
    }, CompatStatus::FULL});
    
    methods.push_back({"playSe", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playSe(
            valueToString(args[0]),
            args.size() > 1 ? static_cast<double>(valueToInt64(args[1])) : 90.0,
            args.size() > 2 ? static_cast<double>(valueToInt64(args[2])) : 100.0);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"stopSe", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopSe();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"playBgs", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playBgs(
            valueToString(args[0]),
            args.size() > 1 ? static_cast<double>(valueToInt64(args[1])) : 90.0,
            args.size() > 2 ? static_cast<double>(valueToInt64(args[2])) : 100.0);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"stopBgs", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopBgs();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"playMe", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playMe(
            valueToString(args[0]),
            args.size() > 1 ? static_cast<double>(valueToInt64(args[1])) : 90.0,
            args.size() > 2 ? static_cast<double>(valueToInt64(args[2])) : 100.0);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"stopMe", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopMe();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"crossfadeBgm", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().crossfadeBgm(
            valueToString(args[0]),
            args.size() > 1 ? static_cast<double>(valueToInt64(args[1])) : 90.0,
            args.size() > 2 ? static_cast<double>(valueToInt64(args[2])) : 100.0,
            args.size() > 3 ? static_cast<int32_t>(valueToInt64(args[3])) : 60);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"saveBgmSettings", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().saveBgmSettings();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"restoreBgmSettings", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().restoreBgmSettings();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("AudioManager", methods);
}

} // namespace compat
} // namespace urpg
