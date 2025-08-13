#include <AL/al.h>
#include <AL/alc.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

constexpr int sampleRate = 44100;

// Generate a sine wave buffer with given frequency and duration (seconds)
short* generateSineWave(float frequency, float duration, int& outSampleCount) {
    outSampleCount = static_cast<int>(sampleRate * duration);
    short* samples = new short[outSampleCount];
    for (int i = 0; i < outSampleCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        samples[i] = static_cast<short>(32760 * sin(2 * M_PI * frequency * t));
    }
    return samples;
}

int main() {
    // Init device and context
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open device\n";
        return -1;
    }
    ALCcontext* context = alcCreateContext(device, nullptr);
    if (!alcMakeContextCurrent(context)) {
        std::cerr << "Failed to make context current\n";
        return -1;
    }

    // Generate buffers and sources for two sounds
    ALuint buffers[2];
    ALuint sources[2];
    alGenBuffers(2, buffers);
    alGenSources(2, sources);

    // Generate two sine waves
    int sampleCount1, sampleCount2;
    short* wave1 = generateSineWave(440.0f, 3.0f, sampleCount1); // A4 for 3 seconds
    short* wave2 = generateSineWave(660.0f, 5.0f, sampleCount2); // E5 for 5 seconds

    // Fill buffers
    alBufferData(buffers[0], AL_FORMAT_MONO16, wave1, sampleCount1 * sizeof(short), sampleRate);
    alBufferData(buffers[1], AL_FORMAT_MONO16, wave2, sampleCount2 * sizeof(short), sampleRate);

    delete[] wave1;
    delete[] wave2;

    // Attach buffers to sources
    alSourcei(sources[0], AL_BUFFER, buffers[0]);
    alSourcei(sources[1], AL_BUFFER, buffers[1]);

    // Source 0: play once, normal pitch, positioned left
    alSourcei(sources[0], AL_LOOPING, AL_FALSE);
    alSource3f(sources[0], AL_POSITION, -2.0f, 0.0f, 0.0f);
    alSourcef(sources[0], AL_PITCH, 1.0f);

    // Source 1: loop, pitch starts normal, positioned right
    alSourcei(sources[1], AL_LOOPING, AL_TRUE);
    alSource3f(sources[1], AL_POSITION, 2.0f, 0.0f, 0.0f);
    alSourcef(sources[1], AL_PITCH, 1.0f);

    // Play both sources simultaneously
    alSourcePlay(sources[0]);
    alSourcePlay(sources[1]);

    std::cout << "Playing two simultaneous sounds...\n";

    // Over 5 seconds, gradually increase pitch of second sound to 1.5x
    for (int i = 0; i <= 50; ++i) {
        float pitch = 1.0f + (i / 50.0f) * 0.5f; // 1.0 to 1.5
        alSourcef(sources[1], AL_PITCH, pitch);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop sources
    alSourceStop(sources[0]);
    alSourceStop(sources[1]);

    // Clean up
    alDeleteSources(2, sources);
    alDeleteBuffers(2, buffers);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    std::cout << "Finished playing complex sounds.\n";
    return 0;
}
