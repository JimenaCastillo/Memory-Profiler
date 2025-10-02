#include "../include/MemoryChart.hpp"
#include "../Library/include/ProfilerNew.hpp"

MemoryChart::MemoryChart(QWidget* parent)
    : QChartView(parent),
      series_(MP_NEW_FT(QLineSeries)),
      chart_(MP_NEW_FT(QChart)),
      axisX_(MP_NEW_FT(QValueAxis)),
      axisY_(MP_NEW_FT(QValueAxis)),
      pointCount_(0)
{
    chart_->addSeries(series_);
    chart_->addAxis(axisX_, Qt::AlignBottom);
    chart_->addAxis(axisY_, Qt::AlignLeft);

    series_->attachAxis(axisX_);
    series_->attachAxis(axisY_);

    axisX_->setRange(0, 50);  // muestra los últimos 50 puntos
    axisY_->setRange(0, 1500); // ajustar según el rango de memoria

    chart_->legend()->hide();
    chart_->setTitle("Historial de uso de memoria");

    setChart(chart_);
    setRenderHint(QPainter::Antialiasing);
}

void MemoryChart::addDataPoint(qreal value) {
    series_->append(pointCount_, value);
    pointCount_++;

    if (pointCount_ > 50) {
        axisX_->setRange(pointCount_ - 50, pointCount_);
    } else {
        axisX_->setRange(0, 50);
    }

    // Escalado dinámico del eje Y
    if (value > axisY_->max()) {
        axisY_->setMax(value * 1.2);  // margen superior
    }

    if (value < axisY_->min()) {
        axisY_->setMin(value * 0.8);  // margen inferior
    }
}