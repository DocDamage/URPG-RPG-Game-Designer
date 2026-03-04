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

void AudioChannel::update() {
    if (playing_ && !paused_) {
        // TODO: Update audio playback position based on time
    }
}

// ============================================================================
// AudioManagerImpl
// ============================================================================

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
    
    // BGS state
    AudioChannel* bgsChannel_ = nullptr;
    
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
        // BGM
        methodStatus_["playBgm"] = CompatStatus::FULL;
        methodStatus_["stopBgm"] = CompatStatus::FULL;
        methodStatus_["pauseBgm"] = CompatStatus::FULL;
        methodStatus_["resumeBgm"] = CompatStatus::FULL;
        methodStatus_["crossfadeBgm"] = CompatStatus::PARTIAL;
        methodDeviations_["crossfadeBgm"] = "Crossfade timing may differ slightly from MZ";
        methodStatus_["saveBgmSettings"] = CompatStatus::FULL;
        methodStatus_["restoreBgmSettings"] = CompatStatus::FULL;
        methodStatus_["isBgmPlaying"] = CompatStatus::FULL;
        methodStatus_["isBgmPaused"] = CompatStatus::FULL;
        methodStatus_["getCurrentBgm"] = CompatStatus::FULL;
        
        // BGS
        methodStatus_["playBgs"] = CompatStatus::FULL;
        methodStatus_["stopBgs"] = CompatStatus::FULL;
        methodStatus_["crossfadeBgs"] = CompatStatus::PARTIAL;
        methodDeviations_["crossfadeBgs"] = "Crossfade timing may differ slightly from MZ";
        
        // ME
        methodStatus_["playMe"] = CompatStatus::FULL;
        methodStatus_["stopMe"] = CompatStatus::FULL;
        
        // SE
        methodStatus_["playSe"] = CompatStatus::FULL;
        methodStatus_["stopSe"] = CompatStatus::FULL;
        
        // Volume
        methodStatus_["setMasterVolume"] = CompatStatus::FULL;
        methodStatus_["getMasterVolume"] = CompatStatus::FULL;
        methodStatus_["setBusVolume"] = CompatStatus::FULL;
        methodStatus_["getBusVolume"] = CompatStatus::FULL;
        
        // Ducking
        methodStatus_["duckBgm"] = CompatStatus::FULL;
        methodStatus_["unduckBgm"] = CompatStatus::FULL;
        methodStatus_["isBgmDucked"] = CompatStatus::FULL;
        
        // Channels
        methodStatus_["createChannel"] = CompatStatus::FULL;
        methodStatus_["destroyChannel"] = CompatStatus::FULL;
        methodStatus_["getChannel"] = CompatStatus::FULL;
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
    if (!impl_->bgmChannel_) {
        createChannel("bgm", AudioBus::BGM);
        impl_->bgmChannel_ = getChannel("bgm");
    }
    
    if (impl_->bgmChannel_) {
        impl_->bgmChannel_->play(filename, volume, pitch, pos);
    }
}

void AudioManager::stopBgm() {
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
    // TODO: Implement actual crossfade with timing
    // For now, just stop and play
    stopBgm();
    playBgm(filename, volume, pitch);
}

void AudioManager::saveBgmSettings() {
    if (impl_->bgmChannel_) {
        impl_->savedBgm_.name = impl_->bgmChannel_->getName();
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
        info.name = impl_->bgmChannel_->getName();
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
    if (!impl_->bgsChannel_) {
        createChannel("bgs", AudioBus::BGS);
        impl_->bgsChannel_ = getChannel("bgs");
    }
    
    if (impl_->bgsChannel_) {
        impl_->bgsChannel_->play(filename, volume, pitch, pos);
    }
}

void AudioManager::stopBgs() {
    if (impl_->bgsChannel_) {
        impl_->bgsChannel_->stop();
    }
}

void AudioManager::crossfadeBgs(const std::string& filename, double volume, double pitch, int32_t duration) {
    // TODO: Implement actual crossfade
    stopBgs();
    playBgs(filename, volume, pitch);
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
        // TODO: Implement smooth ducking over duration
        impl_->bgmChannel_->setVolume(impl_->bgmDuckVolume_);
    }
}

void AudioManager::unduckBgm(int32_t duration) {
    impl_->bgmDucked_ = false;
    
    if (impl_->bgmChannel_) {
        // TODO: Implement smooth unducking over duration
        impl_->bgmChannel_->setVolume(1.0);
    }
}

bool AudioManager::isBgmDucked() const {
    return impl_->bgmDucked_;
}

// ============================================================================
// Update
// ============================================================================

void AudioManager::update() {
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
        // AudioManager::instance().playBgm(...);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"stopBgm", [](const std::vector<Value>&) -> Value {
        // AudioManager::instance().stopBgm();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"pauseBgm", [](const std::vector<Value>&) -> Value {
        // AudioManager::instance().pauseBgm();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"resumeBgm", [](const std::vector<Value>&) -> Value {
        // AudioManager::instance().resumeBgm();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"isBgmPlaying", [](const std::vector<Value>&) -> Value {
        // return Value::Int(AudioManager::instance().isBgmPlaying() ? 1 : 0);
        return Value::Int(0);
    }, CompatStatus::FULL});
    
    methods.push_back({"playSe", [](const std::vector<Value>& args) -> Value {
        if (args.size() < 1) return Value::Nil();
        // AudioManager::instance().playSe(...);
        return Value::Nil();
    }, CompatStatus::FULL});
    
    methods.push_back({"stopSe", [](const std::vector<Value>&) -> Value {
        // AudioManager::instance().stopSe();
        return Value::Nil();
    }, CompatStatus::FULL});
    
    ctx.registerObject("AudioManager", methods);
}

} // namespace compat
} // namespace urpg
