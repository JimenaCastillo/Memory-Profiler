#include "Charts.hpp"

// Qt Charts
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

// Qt base
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>

namespace mp::gui {

    // Constructor: inicializa el gráfico y sus componentes
    Charts::Charts(QWidget* parent)
        : QWidget(parent),
          series(new QLineSeries(this)),
          chart(new QChart()),
          chartView(new QChartView(chart, this))
    {
        // Configurar la serie de datos
        chart->addSeries(series);
        chart->setTitle("Uso de Memoria Activa (active_bytes)");
        chart->legend()->hide(); // Ocultar leyenda (solo una serie)

        // Eje X: tiempo en milisegundos
        axisX = new QValueAxis(this);
        axisX->setTitleText("Tiempo (ms)");
        axisX->setLabelFormat("%lld");
        axisX->setTickCount(10);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        // Eje Y: memoria activa en bytes
        axisY = new QValueAxis(this);
        axisY->setTitleText("Memoria activa (bytes)");
        axisY->setLabelFormat("%lld");
        axisY->setTickCount(10);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        // Activar suavizado de renderizado
        chartView->setRenderHint(QPainter::Antialiasing);

        // Layout vertical para contener el gráfico
        auto layout = new QVBoxLayout(this);
        layout->addWidget(chartView);
        setLayout(layout);
    }

    // Slot que recibe métricas y actualiza el gráfico
    void Charts::updateMemoryUsage(const mp::gui::Metrics& m) {
        // Agregar nuevo punto a la serie
        series->append(static_cast<qreal>(m.t_ms), static_cast<qreal>(m.active_bytes));

        // Ajustar eje X dinámicamente (últimos 60 segundos)
        auto xMax = m.t_ms;
        auto xMin = (xMax > 60000) ? xMax - 60000 : 0;
        axisX->setRange(static_cast<qreal>(xMin), static_cast<qreal>(xMax));

        // Ajustar eje Y según el valor máximo observado
        auto yMax = std::max<std::uint64_t>(m.active_bytes * 1.2, 1000);
        axisY->setRange(0.0, static_cast<qreal>(yMax));
    }

} // namespace mp::gui

