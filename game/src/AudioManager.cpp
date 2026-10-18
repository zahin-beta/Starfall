#include "AudioManager.h"
#include <cmath>
#include <cstring>
#include <cstdint>
#include <initializer_list>

namespace {
constexpr int kSampleRate = 44100;
constexpr double kPi = 3.14159265358979323846;

AudioManager* gMusicInstance = nullptr;

float ClampUnit(float value) {
    return value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
}

short ToPcm(float value, float gain) {
    return static_cast<short>(ClampUnit(value * gain) * 32767.0f);
}

Wave MakeWave(const std::vector<short>& pcm) {
    Wave wave{};
    wave.frameCount = static_cast<unsigned int>(pcm.size());
    wave.sampleRate = static_cast<unsigned int>(kSampleRate);
    wave.sampleSize = 16;
    wave.channels = 1;
    if (!pcm.empty()) {
        short* data = static_cast<short*>(MemAlloc(pcm.size() * sizeof(short)));
        std::memcpy(data, pcm.data(), pcm.size() * sizeof(short));
        wave.data = data;
    }
    return wave;
}

std::vector<short> RenderSweepTone(float startFreq, float endFreq, float duration, float decay, float squareMix) {
    const int count = static_cast<int>(kSampleRate * duration);
    std::vector<short> pcm(count);
    double phase = 0.0;
    for (int i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(count);
        const float freq = startFreq + (endFreq - startFreq) * t;
        phase += 2.0 * kPi * freq / kSampleRate;
        const float sine = static_cast<float>(std::sin(phase));
        const float square = sine >= 0.0f ? 1.0f : -1.0f;
        const float sample = sine * (1.0f - squareMix) + square * squareMix;
        const float attack = 1.0f - std::exp(-120.0f * t);
        const float envelope = std::exp(-decay * t) * attack;
        pcm[i] = ToPcm(sample * envelope, 0.6f);
    }
    return pcm;
}

std::vector<short> RenderArpeggio(std::initializer_list<float> freqs, float noteDuration, float decay) {
    std::vector<short> pcm;
    double phase = 0.0;
    const int count = static_cast<int>(kSampleRate * noteDuration);
    for (float freq : freqs) {
        for (int i = 0; i < count; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(count);
            phase += 2.0 * kPi * freq / kSampleRate;
            const float sample = static_cast<float>(std::sin(phase)) + 0.3f * static_cast<float>(std::sin(phase * 2.0));
            const float envelope = std::exp(-decay * t) * (1.0f - std::exp(-200.0f * t));
            pcm.push_back(ToPcm(sample * envelope, 0.5f));
        }
    }
    return pcm;
}

std::vector<short> RenderExplosion(float duration) {
    const int count = static_cast<int>(kSampleRate * duration);
    std::vector<short> pcm(count);
    uint32_t state = 0x1234567u;
    for (int i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(count);
        state = state * 1664525u + 1013904223u;
        const float noise = static_cast<float>((state >> 9) & 0xFFFF) / 32768.0f - 1.0f;
        const float rumble = static_cast<float>(std::sin(2.0 * kPi * 90.0 * i / kSampleRate));
        const float envelope = std::exp(-5.0f * t) * (1.0f - std::exp(-300.0f * t));
        pcm[i] = ToPcm(noise * 0.75f + rumble * 0.35f, envelope * 0.7f);
    }
    return pcm;
}

float NoteEnvelope(float t, float duration, float attack, float release) {
    if (t < 0.0f || t >= duration) return 0.0f;
    const float a = attack <= 0.0f ? 1.0f : std::min(1.0f, t / attack);
    const float r = release <= 0.0f ? 1.0f : std::min(1.0f, (duration - t) / release);
    return a * r;
}

void AddMusicNote(std::vector<float>& buffer, float startSec, float duration, float freq, float amp, float attack, float release) {
    const int totalFrames = static_cast<int>(buffer.size() / 2);
    const int start = static_cast<int>(startSec * kSampleRate);
    const int count = static_cast<int>(duration * kSampleRate);
    double phase = 0.0;
    for (int i = 0; i < count; ++i) {
        const int index = start + i;
        if (index < 0 || index >= totalFrames) continue;
        const float tSec = static_cast<float>(i) / kSampleRate;
        const float envelope = NoteEnvelope(tSec, duration, attack, release);
        phase += 2.0 * kPi * freq / kSampleRate;
        const float sample = static_cast<float>(std::sin(phase)) * amp * envelope;
        buffer[index * 2] += sample;
        buffer[index * 2 + 1] += sample;
    }
}

std::vector<float> MakeMusicLoop() {
    const float loopDuration = 12.0f;
    const int frames = static_cast<int>(kSampleRate * loopDuration);
    std::vector<float> buffer(frames * 2, 0.0f);

    const float padFreqs[3] = {110.0f, 220.0f, 3956.0f / 12.0f};
    for (int i = 0; i < frames; ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        float pad = 0.0f;
        for (float freq : padFreqs) pad += static_cast<float>(std::sin(2.0 * kPi * freq * t));
        const float tremolo = 0.65f + 0.35f * 0.5f * (1.0f + static_cast<float>(std::sin(2.0 * kPi * (3.0f / loopDuration) * t)));
        pad *= 0.075f * tremolo;
        buffer[i * 2] += pad;
        buffer[i * 2 + 1] += pad;
    }

    const float bassFreqs[4] = {110.0f, 110.0f, 146.832f, 98.0f};
    for (int b = 0; b < 4; ++b) AddMusicNote(buffer, b * 3.0f, 2.6f, bassFreqs[b], 0.14f, 0.02f, 1.2f);

    const float arp[8] = {440.0f, 523.25f, 659.25f, 523.25f, 587.33f, 523.25f, 659.25f, 783.99f};
    const float step = 0.3333333f;
    const int total = static_cast<int>(loopDuration / step);
    for (int n = 0; n < total; ++n) {
        const float start = n * step;
        if (start + step > loopDuration) break;
        AddMusicNote(buffer, start, step * 0.95f, arp[n % 8], 0.06f, 0.01f, step * 0.5f);
    }

    return buffer;
}
}

void AudioManager::MusicCallback(void* bufferData, unsigned int frames) {
    float* output = static_cast<float*>(bufferData);
    if (gMusicInstance == nullptr || gMusicInstance->musicLoop.empty()) {
        for (unsigned int i = 0; i < frames * 2; ++i) output[i] = 0.0f;
        return;
    }
    AudioManager* self = gMusicInstance;
    const std::vector<float>& loop = self->musicLoop;
    const unsigned int totalFrames = static_cast<unsigned int>(loop.size() / 2);
    unsigned int frame = self->musicCursor;
    for (unsigned int i = 0; i < frames; ++i) {
        if (frame >= totalFrames) frame = 0;
        output[i * 2] = loop[frame * 2];
        output[i * 2 + 1] = loop[frame * 2 + 1];
        ++frame;
    }
    self->musicCursor = frame % totalFrames;
}

void AudioManager::Initialize() {
    InitAudioDevice();
    ready = IsAudioDeviceReady();
    if (!ready) return;

    auto toSound = [](const std::vector<short>& pcm) {
        Wave wave = MakeWave(pcm);
        Sound sound = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return sound;
    };

    shootSounds[0] = toSound(RenderSweepTone(950.0f, 520.0f, 0.12f, 9.0f, 0.55f));
    shootSounds[1] = toSound(RenderSweepTone(1040.0f, 560.0f, 0.12f, 9.0f, 0.55f));
    shootSounds[2] = toSound(RenderSweepTone(1130.0f, 610.0f, 0.12f, 9.0f, 0.55f));
    explosionSound = toSound(RenderExplosion(0.35f));
    hitSound = toSound(RenderSweepTone(440.0f, 180.0f, 0.25f, 6.0f, 0.4f));
    gameOverSound = toSound(RenderArpeggio({440.0f, 349.23f, 261.63f, 196.0f}, 0.18f, 5.0f));
    startSound = toSound(RenderArpeggio({330.0f, 440.0f, 587.33f}, 0.08f, 8.0f));

    for (Sound& sound : shootSounds) SetSoundVolume(sound, 0.45f);
    SetSoundVolume(explosionSound, 0.5f);
    SetSoundVolume(hitSound, 0.55f);
    SetSoundVolume(gameOverSound, 0.5f);
    SetSoundVolume(startSound, 0.5f);

    musicLoop = MakeMusicLoop();
    musicCursor = 0;
    gMusicInstance = this;
    SetAudioStreamBufferSizeDefault(4096);
    musicStream = LoadAudioStream(kSampleRate, 32, 2);
    SetAudioStreamCallback(musicStream, MusicCallback);
    SetAudioStreamVolume(musicStream, 0.5f);
}

void AudioManager::Shutdown() {
    if (!ready) return;
    for (Sound& sound : shootSounds) UnloadSound(sound);
    UnloadSound(explosionSound);
    UnloadSound(hitSound);
    UnloadSound(gameOverSound);
    UnloadSound(startSound);
    gMusicInstance = nullptr;
    UnloadAudioStream(musicStream);
    CloseAudioDevice();
    ready = false;
}

void AudioManager::Update() {}

void AudioManager::PlayShoot() {
    if (!ready) return;
    PlaySound(shootSounds[GetRandomValue(0, kShootVariants - 1)]);
}

void AudioManager::PlayExplosion() {
    if (!ready) return;
    PlaySound(explosionSound);
}

void AudioManager::PlayHit() {
    if (!ready) return;
    PlaySound(hitSound);
}

void AudioManager::PlayGameOver() {
    if (!ready) return;
    PlaySound(gameOverSound);
}

void AudioManager::PlayStart() {
    if (!ready) return;
    PlaySound(startSound);
}

void AudioManager::StartMusic() {
    if (!ready) return;
    if (!IsAudioStreamPlaying(musicStream)) PlayAudioStream(musicStream);
}

void AudioManager::StopMusic() {
    if (!ready) return;
    StopAudioStream(musicStream);
}

void AudioManager::SetMusicVolume(float volume) {
    if (!ready) return;
    SetAudioStreamVolume(musicStream, volume);
}
