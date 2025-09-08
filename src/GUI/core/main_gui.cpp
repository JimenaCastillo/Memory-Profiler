#include "GUI.hpp"
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <iostream>

using namespace std::chrono_literals;

static std::atomic<bool> g_running{true};

static void handle_sigint(int) {
    g_running = false;
}

int main() {
    std::signal(SIGINT, handle_sigint);   // permite salir con Ctrl+C

    if (!mp::gui::startGUI(7777)) {
        std::cerr << "[main] Failed to start GUI server\n";
        return 1;
    }
    std::cout << "[main] GUI running on port 7777. Press Ctrl+C to exit.\n";

    while (g_running.load()) {
        mp::gui::tickGUI();
        std::this_thread::sleep_for(50ms);
    }

    mp::gui::stopGUI();
    std::cout << "[main] Bye!\n";
    return 0;
}
