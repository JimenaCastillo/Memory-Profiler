#pragma once
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class MemoryChart : public QChartView {
    Q_OBJECT
public:
    explicit MemoryChart(QWidget* parent = nullptr);
    void addDataPoint(qreal value);

private:
    QLineSeries* series_;
    QChart* chart_;
    QValueAxis* axisX_;
    QValueAxis* axisY_;
    int pointCount_;
};
