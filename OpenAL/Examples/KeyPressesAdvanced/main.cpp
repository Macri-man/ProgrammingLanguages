#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <random>

// === Synth settings ===
constexpr int sampleRate = 44100;
enum Waveform { SINE, SQUARE, SAW, NOISE };

// Generate waveform samples
std::vector<short> generateWaveform(Waveform type, float freq, float duration) {
    int samplesCount = static_cast<int>(sampleRate * duration);
    std::vector<short> samples(samplesCount);
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> noiseDist(-32767, 32767);

    for (int i = 0; i < samplesCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float value = 0.0f;
        switch (type) {
            case SINE: value = sin(2.0f * M_PI * freq * t); break;
            case SQUARE: value = (fmod(freq * t, 1.0f) < 0.5f) ? 1.0f : -1.0f; break;
            case SAW: value = 2.0f * (fmod(freq * t, 1.0f)) - 1.0f; break;
            case NOISE: value = noiseDist(rng) / 32767.0f; break;
        }
        samples[i] = static_cast<short>(value * 32760);
    }
    return samples;
}

// Key to frequency mapping
std::unordered_map<int, float> keyFreq = {
    {GLFW_KEY_A, 440.0f}, // A4
    {GLFW_KEY_S, 494.0f}, // B4
    {GLFW_KEY_D, 523.25f}, // C5
    {GLFW_KEY_F, 587.33f}, // D5
    {GLFW_KEY_G, 659.25f}  // E5
};

// Currently playing notes: key -> source
std::unordered_map<int, ALuint> activeNotes;

int main() {
    // === Init GLFW ===
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenAL Synth", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // === Init ImGui ===
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    // === Init OpenAL ===
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) { std::cerr << "No OpenAL device\n"; return -1; }
    ALCcontext* context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);

    // EFX setup
    bool reverbOn = false, chorusOn = false;
    ALuint effectReverb, effectChorus;
    ALuint slotReverb, slotChorus;
    alGenEffects(1, &effectReverb);
    alGenEffects(1, &effectChorus);
    alGenAuxiliaryEffectSlots(1, &slotReverb);
    alGenAuxiliaryEffectSlots(1, &slotChorus);

    alEffecti(effectReverb, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(effectReverb, AL_REVERB_DECAY_TIME, 2.5f);
    alEffectf(effectReverb, AL_REVERB_GAIN, 0.8f);

    alEffecti(effectChorus, AL_EFFECT_TYPE, AL_EFFECT_CHORUS);
    alEffectf(effectChorus, AL_CHORUS_RATE, 1.5f);
    alEffectf(effectChorus, AL_CHORUS_DEPTH, 0.5f);

    alAuxiliaryEffectSloti(slotReverb, AL_EFFECTSLOT_EFFECT, effectReverb);
    alAuxiliaryEffectSloti(slotChorus, AL_EFFECTSLOT_EFFECT, effectChorus);

    Waveform currentWave = SINE;

    // === Main loop ===
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // === Key handling for sustain ===
        for (auto& [key, freq] : keyFreq) {
            int state = glfwGetKey(window, key);
            if (state == GLFW_PRESS && activeNotes.find(key) == activeNotes.end()) {
                auto samples = generateWaveform(currentWave, freq, 1.0f); // 1 sec buffer
                ALuint buffer, source;
                alGenBuffers(1, &buffer);
                alBufferData(buffer, AL_FORMAT_MONO16, samples.data(),
                             samples.size() * sizeof(short), sampleRate);

                alGenSources(1, &source);
                alSourcei(source, AL_BUFFER, buffer);
                alSourcei(source, AL_LOOPING, AL_TRUE);

                if (reverbOn) alSource3i(source, AL_AUXILIARY_SEND_FILTER, slotReverb, 0, AL_FILTER_NULL);
                if (chorusOn) alSource3i(source, AL_AUXILIARY_SEND_FILTER, slotChorus, 1, AL_FILTER_NULL);

                alSourcePlay(source);
                activeNotes[key] = source;
            }
            else if (state == GLFW_RELEASE && activeNotes.find(key) != activeNotes.end()) {
                ALuint src = activeNotes[key];
                ALint buf;
                alGetSourcei(src, AL_BUFFER, &buf);
                alSourceStop(src);
                alDeleteSources(1, &src);
                alDeleteBuffers(1, reinterpret_cast<ALuint*>(&buf));
                activeNotes.erase(key);
            }
        }

        // === ImGui UI ===
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Synth Controls");
        const char* waveNames[] = { "Sine", "Square", "Saw", "Noise" };
        int waveIdx = static_cast<int>(currentWave);
        if (ImGui::Combo("Waveform", &waveIdx, waveNames, IM_ARRAYSIZE(waveNames))) {
            currentWave = static_cast<Waveform>(waveIdx);
        }
        ImGui::Checkbox("Reverb", &reverbOn);
        ImGui::Checkbox("Chorus", &chorusOn);
        ImGui::Text("Hold A S D F G for notes");
        ImGui::End();

        // Render UI
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // === Cleanup ===
    for (auto& [key, src] : activeNotes) {
        ALint buf;
        alGetSourcei(src, AL_BUFFER, &buf);
        alSourceStop(src);
        alDeleteSources(1, &src);
        alDeleteBuffers(1, reinterpret_cast<ALuint*>(&buf));
    }
    alDeleteAuxiliaryEffectSlots(1, &slotReverb);
    alDeleteAuxiliaryEffectSlots(1, &slotChorus);
    alDeleteEffects(1, &effectReverb);
    alDeleteEffects(1, &effectChorus);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
