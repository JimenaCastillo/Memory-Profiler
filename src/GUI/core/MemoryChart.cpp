#include "../include/MemoryChart.hpp"
#include <QtCharts/QAreaSeries>

MemoryChart::MemoryChart(QWidget* parent)
    : QChartView(parent),
      series_(new QLineSeries()),
      chart_(new QChart()),
      axisX_(new QValueAxis()),
      axisY_(new QValueAxis()),
      pointCount_(0)
{
    // Configurar serie con estilo mejorado
    QPen pen(QColor(100, 180, 255));  // Azul brillante
    pen.setWidth(3);
    series_->setPen(pen);
    series_->setName("Uso de memoria");

    // Crear área bajo la curva para mejor visualización
    QLineSeries* baseSeries = new QLineSeries();
    baseSeries->append(0, 0);

    QAreaSeries* areaSeries = new QAreaSeries(series_, baseSeries);
    areaSeries->setName("Uso de memoria");

    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, QColor(100, 180, 255, 120));  // Azul semi-transparente arriba
    gradient.setColorAt(1.0, QColor(100, 180, 255, 20));   // Casi transparente abajo
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    areaSeries->setBrush(gradient);
    areaSeries->setPen(pen);

    chart_->addSeries(areaSeries);
    chart_->addAxis(axisX_, Qt::AlignBottom);
    chart_->addAxis(axisY_, Qt::AlignLeft);

    areaSeries->attachAxis(axisX_);
    areaSeries->attachAxis(axisY_);

    // Configurar ejes con mejor formato
    axisX_->setRange(0, 50);
    axisX_->setTitleText("Tiempo (muestras)");
    axisX_->setLabelFormat("%d");
    axisX_->setTickCount(11);
    axisX_->setGridLineVisible(true);

    axisY_->setRange(0, 1000);
    axisY_->setTitleText("Memoria (KB)");
    axisY_->setLabelFormat("%.0f");
    axisY_->setTickCount(6);
    axisY_->setGridLineVisible(true);

    // Tema oscuro con mejor contraste
    chart_->setTheme(QChart::ChartThemeDark);
    chart_->setTitle("📈 Historial de Uso de Memoria en Tiempo Real");
    chart_->legend()->setAlignment(Qt::AlignBottom);
    chart_->setAnimationOptions(QChart::SeriesAnimations);

    // Estilo de fondo
    chart_->setBackgroundBrush(QBrush(QColor(30, 30, 30)));
    chart_->setPlotAreaBackgroundBrush(QBrush(QColor(20, 20, 20)));
    chart_->setPlotAreaBackgroundVisible(true);

    setChart(chart_);
    setRenderHint(QPainter::Antialiasing);

    // Mejorar calidad visual
    setStyleSheet("QChartView { border: 2px solid #3a3a3a; border-radius: 8px; }");
}

void MemoryChart::addDataPoint(qreal value) {
    // 1. AGREGAR PUNTO
    series_->append(pointCount_, value);
    pointCount_++;

    // 2. VENTANA DESLIZANTE (últimos 50 puntos)
    if (pointCount_ > 50) {
        axisX_->setRange(pointCount_ - 50, pointCount_);
    } else {
        axisX_->setRange(0, 50);
    }

    // 3. ESCALADO DINÁMICO DEL EJE Y
    qreal maxValue = 0;
    qreal minValue = value;

    // Calcular min/max de los últimos 50 puntos
    int startIdx = qMax(0, pointCount_ - 50);
    for (int i = startIdx; i < pointCount_; ++i) {
        QPointF point = series_->at(i);
        maxValue = qMax(maxValue, point.y());
        minValue = qMin(minValue, point.y());
    }

    // Ajustar rango con márgenes del 10%
    qreal margin = (maxValue - minValue) * 0.1;
    if (margin < 100) margin = 100;  // Margen mínimo de 100 KB

    axisY_->setRange(qMax(0.0, minValue - margin), maxValue + margin);
}