#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>
#include <iostream>
#include <cmath>
#include <thread>
#include <vector>

constexpr int sampleRate = 44100;

short* generateSine(float freq, float duration, int& outSamples) {
    outSamples = static_cast<int>(sampleRate * duration);
    short* samples = new short[outSamples];
    for (int i = 0; i < outSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        samples[i] = static_cast<short>(32767 * sin(2.0 * M_PI * freq * t));
    }
    return samples;
}

int main() {
    // 1. Open device + create context
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) { std::cerr << "Failed to open device\n"; return -1; }
    ALCcontext* context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    // Check EFX support
    if (!alcIsExtensionPresent(device, "ALC_EXT_EFX")) {
        std::cerr << "EFX not supported.\n";
        return -1;
    }

    // 2. Generate sound buffer + source
    ALuint buffer, source;
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    int sampleCount;
    short* data = generateSine(440.0f, 5.0f, sampleCount);
    alBufferData(buffer, AL_FORMAT_MONO16, data, sampleCount * sizeof(short), sampleRate);
    delete[] data;

    alSourcei(source, AL_BUFFER, buffer);

    // 3. Generate EFX objects: effect, slot, filter
    ALuint effectReverb, effectEcho, effectSlotReverb, effectSlotEcho;
    alGenEffects(1, &effectReverb);
    alGenEffects(1, &effectEcho);
    alGenAuxiliaryEffectSlots(1, &effectSlotReverb);
    alGenAuxiliaryEffectSlots(1, &effectSlotEcho);

    // 4. Configure Reverb (preset: Concert Hall)
    alEffecti(effectReverb, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(effectReverb, AL_REVERB_DECAY_TIME, 3.0f);
    alEffectf(effectReverb, AL_REVERB_GAIN, 0.9f);
    alEffectf(effectReverb, AL_REVERB_DENSITY, 1.0f);

    // 5. Configure Echo
    alEffecti(effectEcho, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
    alEffectf(effectEcho, AL_ECHO_DELAY, 0.2f);
    alEffectf(effectEcho, AL_ECHO_LRDELAY, 0.2f);
    alEffectf(effectEcho, AL_ECHO_DAMPING, 0.5f);
    alEffectf(effectEcho, AL_ECHO_FEEDBACK, 0.5f);
    alEffectf(effectEcho, AL_ECHO_SPREAD, -1.0f);

    // 6. Load effects into auxiliary slots
    alAuxiliaryEffectSloti(effectSlotReverb, AL_EFFECTSLOT_EFFECT, effectReverb);
    alAuxiliaryEffectSloti(effectSlotEcho, AL_EFFECTSLOT_EFFECT, effectEcho);

    // 7. Create and attach effect send to source
    ALuint filter;
    alGenFilters(1, &filter);
    alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_NULL); // No filter, just pass through

    // Each source can send audio to multiple effect slots (bus sends)
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, effectSlotReverb, 0, AL_FILTER_NULL);
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, effectSlotEcho, 1, AL_FILTER_NULL);

    // 8. Play sound
    alSourcePlay(source);
    std::cout << "Playing sound with reverb + echo...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 9. Cleanup
    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alDeleteFilters(1, &filter);
    alDeleteAuxiliaryEffectSlots(1, &effectSlotReverb);
    alDeleteAuxiliaryEffectSlots(1, &effectSlotEcho);
    alDeleteEffects(1, &effectReverb);
    alDeleteEffects(1, &effectEcho);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}
