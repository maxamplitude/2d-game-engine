#pragma once
#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
#include <string>
#include <unordered_map>

namespace Engine {

class AudioEngine {
public:
    AudioEngine() = default;
    ~AudioEngine() { shutdown(); }
    
    void init();
    void shutdown();
    
    // Sound effects (loaded into memory, fast playback)
    void loadSound(const std::string& name, const std::string& path);
    void playSound(const std::string& name, float volume = 1.0f);
    
    // Music (streamed from disk, lower memory)
    void loadMusic(const std::string& name, const std::string& path);
    void playMusic(const std::string& name, float volume = 1.0f, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    
    // Master controls
    void setMasterVolume(float volume);  // 0.0 to 1.0
    float getMasterVolume() const;
    
    // Check if playing
    bool isMusicPlaying() const { return currentMusicHandle != -1; }
    
private:
    SoLoud::Soloud soloud;
    std::unordered_map<std::string, SoLoud::Wav> sounds;
    std::unordered_map<std::string, SoLoud::WavStream> music;
    int currentMusicHandle = -1;
};

} // namespace Engine