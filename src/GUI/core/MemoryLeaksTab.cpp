#include "../include/MemoryLeaksTab.hpp"
#include <QHeaderView>
#include <QTableWidgetItem>

MemoryLeaksTab::MemoryLeaksTab(QWidget* parent)
    : QWidget(parent),
      summaryLabel_(new QLabel(this)),
      leakTable_(new QTableWidget(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(summaryLabel_);

    leakTable_->setColumnCount(4);
    leakTable_->setHorizontalHeaderLabels({"Archivo", "Línea", "Tamaño", "Tipo"});
    leakTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(leakTable_);

    barChartView_ = new QChartView(this);
    pieChartView_ = new QChartView(this);
    timeChartView_ = new QChartView(this);

    layout->addWidget(barChartView_);
    layout->addWidget(pieChartView_);
    layout->addWidget(timeChartView_);

}

void MemoryLeaksTab::updateFromLeaks(const LeakSummary& summary) {
    QString text = QString("🔍 Total fugado: %1 KB\n🔢 Total leaks: %2\n📁 Archivo más afectado: %3\n💥 Leak más grande: %4 KB")
        .arg(summary.total_leaked_bytes / 1024.0, 0, 'f', 2)
        .arg(summary.total_leaks)
        .arg(summary.largest_leak_file)
        .arg(summary.largest_leak_size / 1024.0, 0, 'f', 2);

    summaryLabel_->setText(text);

    leakTable_->setRowCount(static_cast<int>(summary.leaks.size()));
    for (int i = 0; i < summary.leaks.size(); ++i) {
        const auto& leak = summary.leaks[i];
        leakTable_->setItem(i, 0, new QTableWidgetItem(leak.file));
        leakTable_->setItem(i, 1, new QTableWidgetItem(QString::number(leak.line)));
        leakTable_->setItem(i, 2, new QTableWidgetItem(QString::number(leak.size / 1024.0, 'f', 2) + " KB"));
        leakTable_->setItem(i, 3, new QTableWidgetItem(leak.type));
    }

    QBarSeries* barSeries = new QBarSeries();
    QBarSet* set = new QBarSet("Fugas");

    QStringList categories;
    for (auto it = summary.leakBytesByFile.begin(); it != summary.leakBytesByFile.end(); ++it) {
        categories << it.key();
        *set << static_cast<qreal>(it.value()) / 1024.0;
    }
    barSeries->append(set);

    QChart* barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("Fugas por archivo (KB)");
    barChart->setAnimationOptions(QChart::SeriesAnimations);

    QCategoryAxis* axisX = new QCategoryAxis();
    for (int i = 0; i < categories.size(); ++i)
        axisX->append(categories[i], i);
    barChart->setAxisX(axisX, barSeries);

    barChartView_->setChart(barChart);

    QPieSeries* pieSeries = new QPieSeries();
    for (auto it = summary.leakBytesByFile.begin(); it != summary.leakBytesByFile.end(); ++it) {
        pieSeries->append(it.key(), static_cast<qreal>(it.value()));
    }

    QChart* pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Distribución de fugas por archivo");
    pieChartView_->setChart(pieChart);

    QLineSeries* timeSeries = new QLineSeries();
    for (const auto& leak : summary.leaks) {
        qint64 t = static_cast<qint64>(leak.timestamp_ns); // Simulación de tiempo
        timeSeries->append(t, static_cast<qreal>(leak.size) / 1024.0);
    }

    QChart* timeChart = new QChart();
    timeChart->addSeries(timeSeries);
    timeChart->setTitle("Fugas en el tiempo (simulado)");
    timeChartView_->setChart(timeChart);
}