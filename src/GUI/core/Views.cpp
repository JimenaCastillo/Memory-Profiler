#include "Views.hpp"

namespace mp::gui {

    // Punteros estáticos a widgets que se actualizarán desde cualquier parte del código
    static QLabel* g_metricsLabel = nullptr;
    static QTableWidget* g_allocTable = nullptr;

    // Guarda las referencias a los widgets que mostrarán métricas
    void initViews(QLabel* metricsLabel, QTableWidget* allocTable) {
        g_metricsLabel = metricsLabel;
        g_allocTable = allocTable;
    }

    // Muestra las métricas iniciales en la etiqueta
    void renderInitialMetrics(const Metrics& m) {
        if (g_metricsLabel) { // Solo si el puntero es válido
            g_metricsLabel->setText(
                QString("Uso actual: %1 B | Pico: %2 B | Activas: %3 | Totales: %4")
                    .arg(m.active_bytes)   // Sustituye %1 por bytes activos
                    .arg(m.peak_bytes)     // Sustituye %2 por pico de bytes
                    .arg(m.active_allocs)  // Sustituye %3 por asignaciones activas
                    .arg(m.total_allocs)   // Sustituye %4 por asignaciones totales
            );
        }
    }

    // Actualiza la tabla con el contador total de asignaciones
    void renderAllocCounter(std::uint64_t total_allocs) {
        if (g_allocTable) { // Solo si el puntero es válido
            g_allocTable->setRowCount(1); // Una sola fila
            // Primera celda: nombre de la métrica
            g_allocTable->setItem(0, 0, new QTableWidgetItem("Total Asignaciones"));
            // Segunda celda: valor numérico
            g_allocTable->setItem(0, 1, new QTableWidgetItem(QString::number(total_allocs)));
        }
    }

} // namespace mp::gui

//unificacion de codigo