#pragma once
#include "raylib.h"
#include <vector>
class AudioManager {
public:
    void Initialize();
    void Shutdown();
    void Update();
    void PlayShoot();
    void PlayExplosion();
    void PlayHit();
    void PlayGameOver();
    void PlayStart();
    void StartMusic();
    void StopMusic();
    void SetMusicVolume(float volume);
private:
    bool ready = false;
    static constexpr int kShootVariants = 3;
    Sound shootSounds[kShootVariants]{};
    Sound explosionSound{};
    Sound hitSound{};
    Sound gameOverSound{};
    Sound startSound{};
    AudioStream musicStream{};
    std::vector<float> musicLoop;
    unsigned int musicCursor = 0;
    static void MusicCallback(void* bufferData, unsigned int frames);
};
