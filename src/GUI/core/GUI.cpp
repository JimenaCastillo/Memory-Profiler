#include "GUI.hpp"
#include "Views.hpp"
#include "Charts.hpp"
#include "SocketServer.hpp"

#include <chrono>
#include <thread>
#include <iostream>

namespace mp::gui {

// Servidor TCP que recibe datos de métricas desde el proceso instrumentado
static SocketServer g_server;

bool startGUI(std::uint16_t port) {
    if (!g_server.start(port)) {
        // Mensaje de error
        std::cerr << "[GUI] Failed to start server.\n";
        return false;
    }

    // Mensaje de estado
    std::cout << "[GUI] Server started on port " << port << ". Waiting metrics...\n";

    // Esperar hasta recibir el primer frame de métricas para inicializar la vista
    for (int i = 0; i < 50; ++i) { // Máximo 5 segundos (50 * 100ms)
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

    // Evitar actualizar más de 2 veces por segundo
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() < 500)
        return;
    last = now;

    // Copiar datos de métricas para graficar
    auto series = g_server.seriesCopy();
    auto latest = g_server.latest();

    // Actualizar contador de asignaciones activas en la GUI
    renderAllocCounter(latest.total_allocs);

    // Actualizar gráfico de uso de memoria
    renderActiveBytesChart(series, 60, 12);

    // Cada 20 ticks (~10 segundos), solicitar un snapshot al servidor
    static int cnt = 0;
    if ((++cnt % 20) == 0)
        g_server.requestSnapshot();

    // Si hay un snapshot disponible, procesarlo
    std::string snap;
    if (g_server.tryPopLastSnapshot(snap)) {
        // Mensaje de estado: opcionalmente mostrar en GUI
        std::cout << "[GUI] Snapshot received (" << snap.size() << " bytes)\n";
    }
}

} // namespace mp::gui
