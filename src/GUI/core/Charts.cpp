#include "Charts.hpp"
#include <string>
#include <iostream>
#include <algorithm>

namespace mp::gui {

// Dibuja un grafico de barras ASCII que muestra la memoria activa en el tiempo
void renderActiveBytesChart(const std::vector<Metrics>& series, int width, int height) {
    // Si no hay datos, no se puede dibujar nada
    if (series.empty()) {
        std::cout << "[Chart] (no data)\n";
        return;
    }

    int n = static_cast<int>(series.size());     // cantidad total de muestras
    int from = std::max(0, n - width);           // empezar desde las ultimas "width" muestras

    // Buscar el valor maximo de active_bytes para escalar la altura del grafico
    std::uint64_t maxy = 1;
    for (int i = from; i < n; ++i) 
        maxy = std::max(maxy, series[i].active_bytes);
    if (maxy == 0) maxy = 1;

    // Crear una "pantalla" (canvas) de altura=height y ancho=(n - from)
    std::vector<std::string> canvas(height, std::string(static_cast<size_t>(n - from), ' '));

    // Rellenar el canvas con asteriscos segun los valores
    for (int i = from; i < n; ++i) {
        std::uint64_t v = series[i].active_bytes; // valor de memoria activa en esta muestra
        // Calcular altura proporcional al valor respecto al maximo
        int h = static_cast<int>((static_cast<long double>(v) * height) / maxy);
        if (h <= 0) h = 1;       // al menos una linea
        if (h > height) h = height; // no pasar la altura maxima

        size_t x = static_cast<size_t>(i - from); // posicion en el eje X
        // Dibujar una columna de asteriscos hacia arriba
        for (int k = 0; k < h; ++k) {
            canvas[height - 1 - k][x] = '*';
        }
    }

    // Mostrar titulo y grafico
    std::cout << "Active Bytes (last " << (n - from) << " samples), max=" << maxy << "B\n";
    for (auto& row : canvas) 
        std::cout << row << "\n";
}

} // namespace mp::gui
