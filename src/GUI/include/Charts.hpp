#pragma once
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include "SocketServer.hpp"

namespace mp::gui {

    // Widget que muestra el gráfico de memoria activa
    class Charts : public QWidget {
        Q_OBJECT

    public:
        explicit Charts(QWidget* parent = nullptr);

    public slots:
        // Slot que recibe métricas en tiempo real
        void updateMemoryUsage(const mp::gui::Metrics& m);

    private:
        QtCharts::QLineSeries* series;
        QtCharts::QChart* chart;
        QtCharts::QChartView* chartView;
    };

} // namespace mp::gui
