// AudioManager - MZ Audio Middleware Compatibility Surface - Implementation
// Phase 2 - Compat Layer

#include "audio_manager.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace urpg {
namespace compat {

namespace {

double ValueToDouble(const Value& value, double fallback) {
    if (const auto* asDouble = std::get_if<double>(&value.v)) {
        return *asDouble;
    }
    if (const auto* asInt = std::get_if<int64_t>(&value.v)) {
        return static_cast<double>(*asInt);
    }
    if (const auto* asBool = std::get_if<bool>(&value.v)) {
        return *asBool ? 1.0 : 0.0;
    }
    return fallback;
}

int32_t ValueToInt(const Value& value, int32_t fallback) {
    if (const auto* asInt = std::get_if<int64_t>(&value.v)) {
        return static_cast<int32_t>(*asInt);
    }
    if (const auto* asDouble = std::get_if<double>(&value.v)) {
        return static_cast<int32_t>(*asDouble);
    }
    if (const auto* asBool = std::get_if<bool>(&value.v)) {
        return *asBool ? 1 : 0;
    }
    return fallback;
}

std::string ValueToString(const Value& value, const std::string& fallback = "") {
    if (const auto* asString = std::get_if<std::string>(&value.v)) {
        return *asString;
    }
    return fallback;
}

AudioBus ValueToBus(const Value& value, AudioBus fallback) {
    if (const auto* asInt = std::get_if<int64_t>(&value.v)) {
        switch (static_cast<AudioBus>(*asInt)) {
            case AudioBus::BGM:
            case AudioBus::BGS:
            case AudioBus::ME:
            case AudioBus::SE:
                return static_cast<AudioBus>(*asInt);
        }
    }
    return fallback;
}

Value AudioInfoToValue(const AudioInfo& info) {
    Object obj;
    obj["name"].v = info.name;
    obj["volume"].v = info.volume;
    obj["pitch"].v = info.pitch;
    obj["pos"].v = static_cast<int64_t>(info.pos);
    return Value::Obj(std::move(obj));
}

double ClampNormalized(double value) {
    return std::clamp(value, 0.0, 1.0);
}

} // namespace

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
    sourceVolume_ = std::clamp(volume / 100.0, 0.0, 1.0);
    volume_ = sourceVolume_;
    pitch_ = std::clamp(pitch / 100.0, 0.5, 2.0);
    pos_ = std::max(0, pos);
    state_ = AudioState::PLAYING;
    playing_ = true;
    paused_ = false;
    framesUntilComplete_ = (bus_ == AudioBus::SE) ? 1 : -1;
}

void AudioChannel::stop() {
    state_ = AudioState::STOPPED;
    playing_ = false;
    paused_ = false;
    pos_ = 0;
    framesUntilComplete_ = -1;
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

void AudioChannel::update() {
    if (playing_ && !paused_) {
        ++pos_;
        if (framesUntilComplete_ > 0) {
            --framesUntilComplete_;
            if (framesUntilComplete_ == 0) {
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

struct PendingVolumeRamp {
    bool active = false;
    double startVolume = 1.0;
    double targetVolume = 1.0;
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
    double bgmBaseVolume_ = 1.0;
    PendingCrossfade bgmCrossfade_;
    PendingVolumeRamp bgmVolumeRamp_;

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

namespace {

double ComputeEffectiveVolume(const AudioManagerImpl& impl,
                              const AudioChannel& channel,
                              double duckMultiplier = 1.0) {
    const auto busVolume = impl.busVolumes_.find(channel.getBus());
    const double bus = busVolume != impl.busVolumes_.end() ? busVolume->second : 1.0;
    return ClampNormalized(channel.getSourceVolume() * impl.masterVolume_ * bus * duckMultiplier);
}

void ApplyChannelVolume(const AudioManagerImpl& impl,
                        AudioChannel* channel,
                        double duckMultiplier = 1.0) {
    if (!channel) {
        return;
    }
    channel->setVolume(ComputeEffectiveVolume(impl, *channel, duckMultiplier));
}

} // namespace

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
        setStatus("playBgm", CompatStatus::PARTIAL,
                  "Drives deterministic harness playback state rather than a live audio backend.");
        setStatus("stopBgm", CompatStatus::PARTIAL,
                  "Stops deterministic harness playback state rather than a live audio backend.");
        setStatus("pauseBgm", CompatStatus::PARTIAL,
                  "Pauses deterministic harness playback state rather than a live audio backend.");
        setStatus("resumeBgm", CompatStatus::PARTIAL,
                  "Resumes deterministic harness playback state rather than a live audio backend.");
        setStatus("crossfadeBgm", CompatStatus::PARTIAL,
                  "Crossfade is deterministic in the harness, but it does not drive a live mixer/backend.");
        setStatus("saveBgmSettings", CompatStatus::PARTIAL,
                  "State snapshot round-trips deterministic harness playback metadata, not a live audio backend state.");
        setStatus("restoreBgmSettings", CompatStatus::PARTIAL,
                  "State restore rehydrates deterministic harness playback metadata, not a live mixer/backend state.");
        setStatus("isBgmPlaying", CompatStatus::PARTIAL,
                  "Reflects deterministic harness playback state rather than a live audio backend.");
        setStatus("isBgmPaused", CompatStatus::PARTIAL,
                  "Reflects deterministic harness playback state rather than a live audio backend.");
        setStatus("getCurrentBgm", CompatStatus::PARTIAL,
                  "Reports deterministic harness playback metadata rather than querying a live audio backend.");
        
        // BGS
        setStatus("playBgs", CompatStatus::PARTIAL,
                  "Drives deterministic harness playback state rather than a live audio backend.");
        setStatus("stopBgs", CompatStatus::PARTIAL,
                  "Stops deterministic harness playback state rather than a live audio backend.");
        setStatus("crossfadeBgs", CompatStatus::PARTIAL,
                  "Crossfade is deterministic in the harness, but it does not drive a live mixer/backend.");
        
        // ME
        setStatus("playMe", CompatStatus::PARTIAL,
                  "Drives deterministic harness playback state rather than a live audio backend.");
        setStatus("stopMe", CompatStatus::PARTIAL,
                  "Stops deterministic harness playback state rather than a live audio backend.");
        
        // SE
        setStatus("playSe", CompatStatus::PARTIAL,
                  "Drives deterministic harness playback state rather than a live audio backend.");
        setStatus("stopSe", CompatStatus::PARTIAL,
                  "Stops deterministic harness playback state rather than a live audio backend.");
        
        // Volume
        setStatus("setMasterVolume", CompatStatus::PARTIAL,
                  "Applies deterministic harness mix scaling rather than a live mixer/backend.");
        setStatus("getMasterVolume", CompatStatus::PARTIAL,
                  "Reports deterministic harness mix scaling rather than a live mixer/backend.");
        setStatus("setBusVolume", CompatStatus::PARTIAL,
                  "Applies deterministic harness mix scaling rather than a live mixer/backend.");
        setStatus("getBusVolume", CompatStatus::PARTIAL,
                  "Reports deterministic harness mix scaling rather than a live mixer/backend.");
        
        // Ducking
        setStatus("duckBgm", CompatStatus::PARTIAL,
                  "Ducking is deterministic in the compat harness, but it does not drive a live mixer/backend.");
        setStatus("unduckBgm", CompatStatus::PARTIAL,
                  "Unducking is deterministic in the compat harness, but it does not drive a live mixer/backend.");
        setStatus("isBgmDucked", CompatStatus::PARTIAL,
                  "Reflects deterministic harness ducking state rather than a live mixer/backend.");
        
        // Channels
        setStatus("createChannel", CompatStatus::PARTIAL,
                  "Creates deterministic harness channels rather than live backend mixer channels.");
        setStatus("destroyChannel", CompatStatus::PARTIAL,
                  "Destroys deterministic harness channels rather than live backend mixer channels.");
        setStatus("getChannel", CompatStatus::PARTIAL,
                  "Returns deterministic harness channels rather than live backend mixer channels.");
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
    impl_->bgmVolumeRamp_.active = false;
    impl_->bgmDucked_ = false;
    impl_->bgmBaseVolume_ = std::clamp(volume / 100.0, 0.0, 1.0);
    if (!impl_->bgmChannel_) {
        createChannel("bgm", AudioBus::BGM);
        impl_->bgmChannel_ = getChannel("bgm");
    }
    
    if (impl_->bgmChannel_) {
        impl_->bgmChannel_->play(filename, volume, pitch, pos);
        ApplyChannelVolume(*impl_, impl_->bgmChannel_);
    }
}

void AudioManager::stopBgm() {
    impl_->bgmCrossfade_.active = false;
    impl_->bgmVolumeRamp_.active = false;
    impl_->bgmDucked_ = false;
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
    impl_->bgmBaseVolume_ = std::clamp(volume / 100.0, 0.0, 1.0);
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
        ApplyChannelVolume(*impl_, impl_->bgsChannel_);
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
        ApplyChannelVolume(*impl_, impl_->meChannel_);
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
        ApplyChannelVolume(*impl_, channel);
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
    ApplyChannelVolume(*impl_, impl_->bgmChannel_);
    ApplyChannelVolume(*impl_, impl_->bgsChannel_);
    ApplyChannelVolume(*impl_, impl_->meChannel_);
    for (auto* channel : impl_->seChannels_) {
        ApplyChannelVolume(*impl_, channel);
    }
}

double AudioManager::getMasterVolume() const {
    return impl_->masterVolume_;
}

void AudioManager::setBusVolume(AudioBus bus, double volume) {
    impl_->busVolumes_[bus] = std::clamp(volume, 0.0, 1.0);
    switch (bus) {
        case AudioBus::BGM:
            ApplyChannelVolume(*impl_, impl_->bgmChannel_);
            break;
        case AudioBus::BGS:
            ApplyChannelVolume(*impl_, impl_->bgsChannel_);
            break;
        case AudioBus::ME:
            ApplyChannelVolume(*impl_, impl_->meChannel_);
            break;
        case AudioBus::SE:
            for (auto* channel : impl_->seChannels_) {
                ApplyChannelVolume(*impl_, channel);
            }
            break;
    }
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
        impl_->bgmVolumeRamp_.startVolume = impl_->bgmChannel_->getVolume();
        impl_->bgmVolumeRamp_.targetVolume = impl_->bgmDuckVolume_;
        impl_->bgmVolumeRamp_.durationFrames = std::max(1, duration);
        impl_->bgmVolumeRamp_.elapsedFrames = 0;
        impl_->bgmVolumeRamp_.active = true;
    }
}

void AudioManager::unduckBgm(int32_t duration) {
    if (impl_->bgmChannel_) {
        impl_->bgmVolumeRamp_.startVolume = impl_->bgmChannel_->getVolume();
        impl_->bgmVolumeRamp_.targetVolume = impl_->bgmBaseVolume_;
        impl_->bgmVolumeRamp_.durationFrames = std::max(1, duration);
        impl_->bgmVolumeRamp_.elapsedFrames = 0;
        impl_->bgmVolumeRamp_.active = true;
    } else {
        impl_->bgmDucked_ = false;
    }
}

bool AudioManager::isBgmDucked() const {
    return impl_->bgmDucked_;
}

// ============================================================================
// Update
// ============================================================================

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

    if (impl_->bgmVolumeRamp_.active && impl_->bgmChannel_) {
        auto& ramp = impl_->bgmVolumeRamp_;
        ramp.elapsedFrames = std::min(ramp.durationFrames, ramp.elapsedFrames + 1);
        const double t = static_cast<double>(ramp.elapsedFrames) / std::max(1, ramp.durationFrames);
        const double volume = ramp.startVolume + ((ramp.targetVolume - ramp.startVolume) * t);
        impl_->bgmChannel_->setVolume(volume);

        if (ramp.elapsedFrames >= ramp.durationFrames) {
            ramp.active = false;
            impl_->bgmChannel_->setVolume(ramp.targetVolume);
            if (std::abs(ramp.targetVolume - impl_->bgmBaseVolume_) < 0.0001) {
                impl_->bgmDucked_ = false;
            }
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

void AudioManager::registerAPI(QuickJSContext& ctx) {
    std::vector<QuickJSContext::MethodDef> methods;
    
    methods.push_back({"playBgm", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playBgm(
            ValueToString(args[0]),
            args.size() > 1 ? ValueToDouble(args[1], 90.0) : 90.0,
            args.size() > 2 ? ValueToDouble(args[2], 100.0) : 100.0,
            args.size() > 3 ? ValueToInt(args[3], 0) : 0);
        return Value::Nil();
    }, AudioManager::getMethodStatus("playBgm"), AudioManager::getMethodDeviation("playBgm")});
    
    methods.push_back({"stopBgm", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopBgm();
        return Value::Nil();
    }, AudioManager::getMethodStatus("stopBgm"), AudioManager::getMethodDeviation("stopBgm")});
    
    methods.push_back({"pauseBgm", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().pauseBgm();
        return Value::Nil();
    }, AudioManager::getMethodStatus("pauseBgm"), AudioManager::getMethodDeviation("pauseBgm")});
    
    methods.push_back({"resumeBgm", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().resumeBgm();
        return Value::Nil();
    }, AudioManager::getMethodStatus("resumeBgm"), AudioManager::getMethodDeviation("resumeBgm")});
    
    methods.push_back({"isBgmPlaying", [](const std::vector<Value>&) -> Value {
        return Value::Int(AudioManager::instance().isBgmPlaying() ? 1 : 0);
    }, AudioManager::getMethodStatus("isBgmPlaying"), AudioManager::getMethodDeviation("isBgmPlaying")});

    methods.push_back({"isBgmPaused", [](const std::vector<Value>&) -> Value {
        return Value::Int(AudioManager::instance().isBgmPaused() ? 1 : 0);
    }, AudioManager::getMethodStatus("isBgmPaused"), AudioManager::getMethodDeviation("isBgmPaused")});

    methods.push_back({"getCurrentBgm", [](const std::vector<Value>&) -> Value {
        return AudioInfoToValue(AudioManager::instance().getCurrentBgm());
    }, AudioManager::getMethodStatus("getCurrentBgm"), AudioManager::getMethodDeviation("getCurrentBgm")});

    methods.push_back({"playBgs", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playBgs(
            ValueToString(args[0]),
            args.size() > 1 ? ValueToDouble(args[1], 90.0) : 90.0,
            args.size() > 2 ? ValueToDouble(args[2], 100.0) : 100.0,
            args.size() > 3 ? ValueToInt(args[3], 0) : 0);
        return Value::Nil();
    }, AudioManager::getMethodStatus("playBgs"), AudioManager::getMethodDeviation("playBgs")});

    methods.push_back({"stopBgs", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopBgs();
        return Value::Nil();
    }, AudioManager::getMethodStatus("stopBgs"), AudioManager::getMethodDeviation("stopBgs")});

    methods.push_back({"playMe", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playMe(
            ValueToString(args[0]),
            args.size() > 1 ? ValueToDouble(args[1], 90.0) : 90.0,
            args.size() > 2 ? ValueToDouble(args[2], 100.0) : 100.0);
        return Value::Nil();
    }, AudioManager::getMethodStatus("playMe"), AudioManager::getMethodDeviation("playMe")});

    methods.push_back({"stopMe", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopMe();
        return Value::Nil();
    }, AudioManager::getMethodStatus("stopMe"), AudioManager::getMethodDeviation("stopMe")});
    
    methods.push_back({"playSe", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        AudioManager::instance().playSe(
            ValueToString(args[0]),
            args.size() > 1 ? ValueToDouble(args[1], 90.0) : 90.0,
            args.size() > 2 ? ValueToDouble(args[2], 100.0) : 100.0);
        return Value::Nil();
    }, AudioManager::getMethodStatus("playSe"), AudioManager::getMethodDeviation("playSe")});
    
    methods.push_back({"stopSe", [](const std::vector<Value>&) -> Value {
        AudioManager::instance().stopSe();
        return Value::Nil();
    }, AudioManager::getMethodStatus("stopSe"), AudioManager::getMethodDeviation("stopSe")});

    methods.push_back({"setMasterVolume", [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::Nil();
        AudioManager::instance().setMasterVolume(ValueToDouble(args[0], 100.0) / 100.0);
        return Value::Nil();
    }, AudioManager::getMethodStatus("setMasterVolume"), AudioManager::getMethodDeviation("setMasterVolume")});

    methods.push_back({"getMasterVolume", [](const std::vector<Value>&) -> Value {
        Value out;
        out.v = AudioManager::instance().getMasterVolume();
        return out;
    }, AudioManager::getMethodStatus("getMasterVolume"), AudioManager::getMethodDeviation("getMasterVolume")});

    methods.push_back({"setBusVolume", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::Nil();
        AudioManager::instance().setBusVolume(
            ValueToBus(args[0], AudioBus::BGM),
            ValueToDouble(args[1], 100.0) / 100.0);
        return Value::Nil();
    }, AudioManager::getMethodStatus("setBusVolume"), AudioManager::getMethodDeviation("setBusVolume")});

    methods.push_back({"getBusVolume", [](const std::vector<Value>& args) -> Value {
        Value out;
        out.v = AudioManager::instance().getBusVolume(args.empty() ? AudioBus::BGM : ValueToBus(args[0], AudioBus::BGM));
        return out;
    }, AudioManager::getMethodStatus("getBusVolume"), AudioManager::getMethodDeviation("getBusVolume")});

    methods.push_back({"duckBgm", [](const std::vector<Value>& args) -> Value {
        AudioManager::instance().duckBgm(
            args.empty() ? 50.0 : ValueToDouble(args[0], 50.0),
            args.size() > 1 ? ValueToInt(args[1], 30) : 30);
        return Value::Nil();
    }, AudioManager::getMethodStatus("duckBgm"), AudioManager::getMethodDeviation("duckBgm")});

    methods.push_back({"unduckBgm", [](const std::vector<Value>& args) -> Value {
        AudioManager::instance().unduckBgm(args.empty() ? 30 : ValueToInt(args[0], 30));
        return Value::Nil();
    }, AudioManager::getMethodStatus("unduckBgm"), AudioManager::getMethodDeviation("unduckBgm")});

    methods.push_back({"isBgmDucked", [](const std::vector<Value>&) -> Value {
        return Value::Int(AudioManager::instance().isBgmDucked() ? 1 : 0);
    }, AudioManager::getMethodStatus("isBgmDucked"), AudioManager::getMethodDeviation("isBgmDucked")});
    
    ctx.registerObject("AudioManager", methods);
}

} // namespace compat
} // namespace urpg
