#include "audio_engine.h"
#include "platform/logging.h"

namespace Engine {

void AudioEngine::init() {
    soloud.init();
    Log::info("SoLoud audio initialized");
}

void AudioEngine::shutdown() {
    soloud.deinit();
    Log::info("SoLoud audio shutdown");
}

void AudioEngine::loadSound(const std::string& name, const std::string& path) {
    SoLoud::Wav wav;
    SoLoud::result result = wav.load(path.c_str());
    
    if (result != SoLoud::SO_NO_ERROR) {
        Log::error("Failed to load sound: {} (error: {})", path, result);
        return;
    }
    
    sounds[name] = std::move(wav);
    Log::info("Sound loaded: {}", name);
}

void AudioEngine::playSound(const std::string& name, float volume) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        soloud.play(it->second, volume);
    } else {
        Log::warn("Sound not found: {}", name);
    }
}

void AudioEngine::loadMusic(const std::string& name, const std::string& path) {
    SoLoud::WavStream stream;
    SoLoud::result result = stream.load(path.c_str());
    
    if (result != SoLoud::SO_NO_ERROR) {
        Log::error("Failed to load music: {} (error: {})", path, result);
        return;
    }
    
    music[name] = std::move(stream);
    Log::info("Music loaded: {}", name);
}

void AudioEngine::playMusic(const std::string& name, float volume, bool loop) {
    auto it = music.find(name);
    if (it != music.end()) {
        stopMusic();  // Stop current music first
        it->second.setLooping(loop);
        currentMusicHandle = soloud.play(it->second, volume);
        Log::info("Playing music: {}", name);
    } else {
        Log::warn("Music not found: {}", name);
    }
}

void AudioEngine::stopMusic() {
    if (currentMusicHandle != -1) {
        soloud.stop(currentMusicHandle);
        currentMusicHandle = -1;
    }
}

void AudioEngine::pauseMusic() {
    if (currentMusicHandle != -1) {
        soloud.setPause(currentMusicHandle, true);
    }
}

void AudioEngine::resumeMusic() {
    if (currentMusicHandle != -1) {
        soloud.setPause(currentMusicHandle, false);
    }
}

void AudioEngine::setMasterVolume(float volume) {
    soloud.setGlobalVolume(volume);
}

float AudioEngine::getMasterVolume() const {
    return soloud.getGlobalVolume();
}

} // namespace Engine