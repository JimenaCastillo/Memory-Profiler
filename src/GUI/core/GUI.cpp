#include "GUI.hpp"
#include "Views.hpp"
#include "Charts.hpp"
#include "SocketServer.hpp"

#include <chrono>
#include <thread>
#include <iostream>

namespace mp::gui {

static SocketServer g_server;

bool startGUI(std::uint16_t port) {
    if (!g_server.start(port)) {
        std::cerr << "[GUI] Failed to start server.\n";
        return false;
    }
    std::cout << "[GUI] Server started on port " << port << ". Waiting metrics...\n";

    // Mostrar métricas iniciales cuando llegue el primer frame
    for (int i = 0; i < 50; ++i) {
        auto m = g_server.latest();
        if (m.peak_bytes || m.active_bytes || m.total_allocs || m.active_allocs) {
            renderInitialMetrics(m);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return true;
}

void stopGUI() {
    g_server.stop();
}

void tickGUI() {
    static auto last = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() < 500) return;
    last = now;

    auto series = g_server.seriesCopy();
    auto latest = g_server.latest();

    renderAllocCounter(latest.total_allocs);
    renderActiveBytesChart(series, 60, 12);

    static int cnt = 0;
    if ((++cnt % 20) == 0) g_server.requestSnapshot();

    std::string snap;
    if (g_server.tryPopLastSnapshot(snap)) {
        std::cout << "[GUI] Snapshot received (" << snap.size() << " bytes)\n";
    }
}

} // namespace mp::gui
