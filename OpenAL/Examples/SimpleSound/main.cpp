#include <AL/al.h>
#include <AL/alc.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 1. Open default device
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open audio device" << std::endl;
        return -1;
    }

    // 2. Create OpenAL context
    ALCcontext* context = alcCreateContext(device, nullptr);
    if (!alcMakeContextCurrent(context)) {
        std::cerr << "Failed to make context current" << std::endl;
        return -1;
    }

    // 3. Generate a buffer and source
    ALuint buffer, source;
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    // 4. Generate a sine wave (440 Hz, 1 second, 16-bit mono, 44.1kHz)
    constexpr int sampleRate = 44100;
    constexpr float frequency = 440.0f; // A4
    constexpr float duration = 1.0f;    // seconds
    constexpr int samplesCount = static_cast<int>(sampleRate * duration);

    short* samples = new short[samplesCount];
    for (int i = 0; i < samplesCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        samples[i] = static_cast<short>(32760 * sin(2 * M_PI * frequency * t));
    }

    // 5. Fill buffer with audio data
    alBufferData(buffer, AL_FORMAT_MONO16, samples, samplesCount * sizeof(short), sampleRate);
    delete[] samples;

    // 6. Attach buffer to source and play
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);

    std::cout << "Playing 440 Hz sine wave for 1 second..." << std::endl;

    // Wait for sound to finish
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 7. Cleanup
    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    std::cout << "Done." << std::endl;
    return 0;
}
