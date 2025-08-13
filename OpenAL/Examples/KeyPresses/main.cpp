#include <AL/al.h>
#include <AL/alc.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <vector>

constexpr int sampleRate = 44100;
constexpr float noteDuration = 0.5f; // seconds

// Generate sine wave samples
std::vector<short> generateSine(float freq, float duration) {
    int sampleCount = static_cast<int>(sampleRate * duration);
    std::vector<short> samples(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        samples[i] = static_cast<short>(32760 * sin(2.0f * M_PI * freq * t));
    }
    return samples;
}

int main() {
    // 1. Init GLFW (for keyboard input)
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(400, 200, "OpenAL Synth", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    // 2. Init OpenAL
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open OpenAL device\n";
        return -1;
    }
    ALCcontext* context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    // 3. Pre-generate note buffers
    std::unordered_map<int, ALuint> noteBuffers;
    std::unordered_map<int, float> keyToFreq = {
        {GLFW_KEY_A, 440.0f},    // A4
        {GLFW_KEY_S, 494.0f},    // B4
        {GLFW_KEY_D, 523.25f},   // C5
        {GLFW_KEY_F, 587.33f},   // D5
        {GLFW_KEY_G, 659.25f}    // E5
    };

    for (auto& [key, freq] : keyToFreq) {
        auto samples = generateSine(freq, noteDuration);
        ALuint buffer;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, AL_FORMAT_MONO16, samples.data(),
                     samples.size() * sizeof(short), sampleRate);
        noteBuffers[key] = buffer;
    }

    std::cout << "Press A S D F G to play notes, ESC to quit.\n";

    // 4. Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        for (auto& [key, freq] : keyToFreq) {
            if (glfwGetKey(window, key) == GLFW_PRESS) {
                // Create a source for each note press (polyphony)
                ALuint source;
                alGenSources(1, &source);
                alSourcei(source, AL_BUFFER, noteBuffers[key]);
                alSourcePlay(source);

                // Detach and delete the source after duration
                std::thread([source]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(
                        static_cast<int>(noteDuration * 1000)));
                    alSourceStop(source);
                    alDeleteSources(1, &source);
                }).detach();
            }
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;
    }

    // 5. Cleanup
    for (auto& [key, buffer] : noteBuffers) {
        alDeleteBuffers(1, &buffer);
    }

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
