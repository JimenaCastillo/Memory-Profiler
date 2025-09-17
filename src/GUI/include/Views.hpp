#pragma once
#include <cstdint>
#include <QLabel>
#include <QTableWidget>
#include "SocketServer.hpp"

namespace mp::gui {

    // Guarda punteros a widgets para poder actualizarlos desde otras funciones
    void initViews(QLabel* metricsLabel, QTableWidget* allocTable);

    // Actualiza la etiqueta con métricas iniciales
    void renderInitialMetrics(const Metrics& m);

    // Actualiza la tabla con el contador de asignaciones
    void renderAllocCounter(std::uint64_t total_allocs);

} // namespace mp::gui

//unificacion de codigo