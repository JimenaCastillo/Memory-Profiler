#pragma once
#include <QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include "SocketServer.hpp"
#include <QWidget>

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
        QLineSeries* series;
        QChart* chart;
        QChartView* chartView;
        QValueAxis* axisX;
        QValueAxis* axisY;
    };

} // namespace mp::gui
