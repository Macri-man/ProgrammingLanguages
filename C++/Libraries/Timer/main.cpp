#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <cstdlib>

using namespace std::chrono_literals;

class Timer {
public:
    explicit Timer(std::chrono::duration<float> duration)
        : duration(duration), running(false) {}

    void start() {
        startTime = std::chrono::high_resolution_clock::now();
        running = true;
    }

    bool isExpired() const {
        if (!running) return false;
        return std::chrono::high_resolution_clock::now() - startTime >= duration;
    }

    void reset() {
        start();
    }

private:
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::duration<float> duration;
    bool running;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <duration_in_seconds>\n";
        return 1;
    }

    float durationSeconds{};
    try {
        durationSeconds = std::stof(argv[1]);
        if (durationSeconds <= 0.0f) throw std::invalid_argument("negative");
    } catch (...) {
        std::cerr << "Invalid duration. Please enter a positive number.\n";
        return 1;
    }

    Timer timer(std::chrono::duration<float>{durationSeconds});
    timer.start();

    while (true) {
        std::cout << "Doing other work...\n";

        if (timer.isExpired()) {
            std::cout << "Timer expired after " << durationSeconds << " seconds!\n";
            break;
        }

        std::this_thread::sleep_for(500ms);  // C++20 chrono literal
    }

    return 0;
}
