#include <AL/al.h>
#include <AL/alc.h>
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <iostream>
#include <algorithm>

// ---------------- Audio config ----------------
static constexpr int   kSampleRate       = 48000;   // use 48k for lower latency on many systems
static constexpr int   kFramesPerChunk   = 512;     // ~10.7ms per chunk at 48k
static constexpr int   kNumQueuedBuffers = 6;       // a small ring to avoid underruns

// ---------------- Synth config ----------------
enum class Waveform { Sine, Square, Saw, Noise };

// Note key bindings
static const std::unordered_map<int, float> kKeyFreqs = {
    {GLFW_KEY_A, 440.00f}, // A4
    {GLFW_KEY_S, 493.88f}, // B4
    {GLFW_KEY_D, 523.25f}, // C5
    {GLFW_KEY_F, 587.33f}, // D5
    {GLFW_KEY_G, 659.25f}, // E5
    {GLFW_KEY_H, 698.46f}, // F5
    {GLFW_KEY_J, 783.99f}, // G5
};

// Per-note phase accumulator to keep continuity between chunks
struct Phase {
    double phase = 0.0; // radians
};

static std::unordered_map<int, Phase> g_phase;

// Visualization ring buffer (float in [-1,1])
struct ScopeRing {
    std::vector<float> data;
    size_t write = 0;
    ScopeRing(size_t n = 4096) : data(n, 0.0f) {}
    void push(float v) {
        data[write] = v;
        write = (write + 1) % data.size();
    }
    // Copy out N most-recent samples into out[]
    void latest(size_t N, std::vector<float>& out) {
        N = std::min(N, data.size());
        out.resize(N);
        size_t start = (write + data.size() - N) % data.size();
        for (size_t i = 0; i < N; ++i)
            out[i] = data[(start + i) % data.size()];
    }
};

static ScopeRing g_scope(8192);

// RNG for noise
static std::mt19937 g_rng{std::random_device{}()};
static std::uniform_real_distribution<float> g_noise(-1.0f, 1.0f);

// ---------------- Wave generation helpers ----------------
inline float fast_sine(double phase) {
    return std::sin(static_cast<float>(phase)); // precise enough; could optimize later
}

inline float next_sample(Waveform wf, double& phase, double phaseInc) {
    float s = 0.0f;
    switch (wf) {
        case Waveform::Sine:   s = fast_sine(phase); break;
        case Waveform::Square: s = (fast_sine(phase) >= 0.0f) ? 1.0f : -1.0f; break;
        case Waveform::Saw: {
            // map phase [0, 2π) to [-1,1): saw rising
            double t = std::fmod(phase, 2.0 * M_PI);
            if (t < 0) t += 2.0 * M_PI;
            s = static_cast<float>((t / (2.0 * M_PI)) * 2.0 - 1.0);
            break;
        }
        case Waveform::Noise:  s = g_noise(g_rng); break;
    }
    // advance phase for oscillatory types
    if (wf != Waveform::Noise) {
        phase += phaseInc;
        if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
    }
    return s;
}

// Mix current held notes into interleaved mono int16 samples
static void generate_chunk_int16(
    const std::unordered_set<int>& activeKeys,
    Waveform wf, float gain,
    std::vector<short>& out_i16)
{
    out_i16.resize(kFramesPerChunk);
    if (activeKeys.empty()) {
        std::fill(out_i16.begin(), out_i16.end(), 0);
        // still push zeros to scope so it decays to silence visually
        for (int i = 0; i < kFramesPerChunk; ++i) g_scope.push(0.0f);
        return;
    }

    // Precompute phase increments and refs for all active keys
    struct Note {
        int key;
        double* phase;
        double inc;
    };
    std::vector<Note> notes;
    notes.reserve(activeKeys.size());
    for (int key : activeKeys) {
        float freq = kKeyFreqs.at(key);
        double inc = (2.0 * M_PI * freq) / double(kSampleRate);
        notes.push_back(Note{key, &g_phase[key].phase, inc});
    }

    float invCount = 1.0f / float(notes.size());
    for (int i = 0; i < kFramesPerChunk; ++i) {
        float mix = 0.0f;
        for (auto& n : notes) {
            mix += next_sample(wf, *n.phase, n.inc);
        }
        // simple average + gain, hard clip to [-1,1]
        float s = std::clamp(mix * invCount * gain, -1.0f, 1.0f);
        out_i16[i] = static_cast<short>(s * 32767.0f);

        // add to scope ring
        g_scope.push(s);
    }
}

// ---------------- OpenAL helpers (streaming) ----------------
struct AlStream {
    ALCdevice*  device  = nullptr;
    ALCcontext* context = nullptr;
    ALuint      source  = 0;
    ALuint      buffers[kNumQueuedBuffers]{};

    bool init() {
        device = alcOpenDevice(nullptr);
        if (!device) return false;
        context = alcCreateContext(device, nullptr);
        if (!context || !alcMakeContextCurrent(context)) return false;

        alGenSources(1, &source);
        alGenBuffers(kNumQueuedBuffers, buffers);
        return true;
    }

    void shutdown() {
        if (source) {
            alSourceStop(source);
            ALint queued = 0;
            alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
            while (queued-- > 0) {
                ALuint b; alSourceUnqueueBuffers(source, 1, &b);
            }
            alDeleteSources(1, &source);
        }
        alDeleteBuffers(kNumQueuedBuffers, buffers);
        if (context) { alcMakeContextCurrent(nullptr); alcDestroyContext(context); }
        if (device) { alcCloseDevice(device); }
    }

    void queue_initial_silence() {
        std::vector<short> silence(kFramesPerChunk, 0);
        for (int i = 0; i < kNumQueuedBuffers; ++i) {
            alBufferData(buffers[i], AL_FORMAT_MONO16, silence.data(),
                         (ALsizei)(silence.size() * sizeof(short)), kSampleRate);
        }
        alSourceQueueBuffers(source, kNumQueuedBuffers, buffers);
        alSourcePlay(source);
    }

    void pump(const std::vector<short>& chunk) {
        ALint processed = 0;
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        while (processed-- > 0) {
            ALuint b = 0;
            alSourceUnqueueBuffers(source, 1, &b);
            alBufferData(b, AL_FORMAT_MONO16, chunk.data(),
                         (ALsizei)(chunk.size() * sizeof(short)), kSampleRate);
            alSourceQueueBuffers(source, 1, &b);
        }
        // If underflowed (stopped), restart
        ALint state = 0;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) alSourcePlay(source);
    }
};

// ---------------- Main ----------------
int main() {
    // GLFW + window
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return 1; }
    GLFWwindow* window = glfwCreateWindow(900, 520, "ImGui + OpenAL Waveform", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // OpenAL stream
    AlStream audio;
    if (!audio.init()) {
        std::cerr << "OpenAL init failed\n";
        return 1;
    }
    audio.queue_initial_silence();

    // UI state
    Waveform wf = Waveform::Sine;
    int wfIdx   = 0;
    float gain  = 0.8f;
    bool showGrid = true;
    int scopeSamples = 2048; // number of samples to display

    std::unordered_set<int> activeKeys;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Poll key states for sustain
        for (auto& kv : kKeyFreqs) {
            int key = kv.first;
            int state = glfwGetKey(window, key);
            if (state == GLFW_PRESS) activeKeys.insert(key);
            else if (state == GLFW_RELEASE) activeKeys.erase(key);
        }

        // Generate next audio chunk and pump it into OpenAL
        std::vector<short> chunk;
        generate_chunk_int16(activeKeys, wf, gain, chunk);
        audio.pump(chunk);

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Synth");
        const char* waves[] = { "Sine", "Square", "Saw", "Noise" };
        if (ImGui::Combo("Waveform", &wfIdx, waves, IM_ARRAYSIZE(waves))) {
            wf = static_cast<Waveform>(wfIdx);
        }
        ImGui::SliderFloat("Gain", &gain, 0.0f, 1.5f, "%.2f");
        ImGui::Checkbox("Scope grid", &showGrid);
        ImGui::SliderInt("Scope samples", &scopeSamples, 256, (int)g_scope.data.size());

        ImGui::Separator();
        ImGui::Text("Hold keys: A S D F G H J (Esc to quit)");

        // Draw oscilloscope
        std::vector<float> scope;
        g_scope.latest((size_t)scopeSamples, scope);

        // Optional simple grid (using background draw list)
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 sz = ImVec2(ImGui::GetContentRegionAvail().x, 180.0f);
        if (showGrid) {
            auto* dl = ImGui::GetWindowDrawList();
            const int vlines = 8, hlines = 4;
            for (int i = 0; i <= vlines; ++i) {
                float x = p.x + (sz.x * i) / (float)vlines;
                dl->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + sz.y), IM_COL32(70,70,70,120));
            }
            for (int i = 0; i <= hlines; ++i) {
                float y = p.y + (sz.y * i) / (float)hlines;
                dl->AddLine(ImVec2(p.x, y), ImVec2(p.x + sz.x, y), IM_COL32(70,70,70,120));
            }
        }

        // Plot waveform (normalized)
        if (!scope.empty()) {
            ImGui::PlotLines("##scope", scope.data(), (int)scope.size(),
                             0, nullptr, -1.0f, 1.0f, sz);
        } else {
            ImGui::Dummy(sz);
        }

        ImGui::End();

        // render
        ImGui::Render();
        int dw, dh; glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
    }

    // Cleanup
    audio.shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
