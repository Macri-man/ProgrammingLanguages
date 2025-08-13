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
    // 1. Open device and create context
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

    // 2. Generate buffer and source
    ALuint buffer, source;
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    // 3. Generate sine wave (440Hz, 10 seconds)
    int sampleCount;
    short* samples = generateSineWave(440.0f, 10.0f, sampleCount);
    alBufferData(buffer, AL_FORMAT_MONO16, samples, sampleCount * sizeof(short), sampleRate);
    delete[] samples;

    // 4. Attach buffer to source
    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_LOOPING, AL_TRUE);

    // 5. Set Doppler parameters (optional tuning)
    alDopplerFactor(1.0f); // default is 1.0, can increase for more pronounced effect
    alDopplerVelocity(343.3f); // speed of sound in m/s (default)

    // 6. Set initial positions and velocities
    // Listener at origin, stationary
    ALfloat listenerPos[] = { 0.0f, 0.0f, 0.0f };
    ALfloat listenerVel[] = { 0.0f, 0.0f, 0.0f };
    ALfloat listenerOri[] = { 0.0f, 0.0f, -1.0f,  // Forward vector
                             0.0f, 1.0f, 0.0f }; // Up vector

    alListenerfv(AL_POSITION, listenerPos);
    alListenerfv(AL_VELOCITY, listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);

    // Source initial position and velocity (moving along +X axis)
    ALfloat sourcePos[] = { -10.0f, 0.0f, 0.0f };
    ALfloat sourceVel[] = { 5.0f, 0.0f, 0.0f }; // 5 m/s toward +X

    alSourcefv(source, AL_POSITION, sourcePos);
    alSourcefv(source, AL_VELOCITY, sourceVel);

    // 7. Play source
    alSourcePlay(source);
    std::cout << "Playing moving sound with Doppler effect...\n";

    // 8. Update source position over 5 seconds
    const int updates = 50;
    const float timeStep = 0.1f; // 100ms per update
    for (int i = 0; i < updates; ++i) {
        // Update source position
        sourcePos[0] += sourceVel[0] * timeStep; // x += vx * dt
        alSourcefv(source, AL_POSITION, sourcePos);

        // For demonstration, let's keep velocity constant.
        // In real app, velocity could change, e.g. slowing down.

        // Sleep 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 9. Cleanup
    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    std::cout << "Finished.\n";
    return 0;
}
