#include "../include/MemoryLeaksTab.hpp"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSplitter>

MemoryLeaksTab::MemoryLeaksTab(QWidget* parent)
    : QWidget(parent),
      summaryLabel_(new QLabel(this)),
      leakTable_(new QTableWidget(this))
{
    auto* mainLayout = new QVBoxLayout(this);

    // ===== PANEL DE RESUMEN MEJORADO =====
    summaryLabel_->setStyleSheet(
        "QLabel { "
        "   background-color: #2b2b2b; "
        "   color: #ffffff; "
        "   padding: 15px; "
        "   border-radius: 8px; "
        "   font-size: 13px; "
        "   font-family: 'Courier New', monospace; "
        "}"
    );
    summaryLabel_->setMinimumHeight(100);
    mainLayout->addWidget(summaryLabel_);

    // ===== TABLA DE LEAKS MEJORADA =====
    leakTable_->setColumnCount(5);
    leakTable_->setHorizontalHeaderLabels({"Archivo", "Línea", "Tamaño (KB)", "Tipo", "Timestamp"});
    leakTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    leakTable_->setAlternatingRowColors(true);
    leakTable_->setStyleSheet(
        "QTableWidget { "
        "   gridline-color: #3a3a3a; "
        "   background-color: #1e1e1e; "
        "   color: #ffffff; "
        "}"
        "QTableWidget::item { "
        "   padding: 5px; "
        "}"
        "QTableWidget::item:alternate { "
        "   background-color: #252525; "
        "}"
        "QHeaderView::section { "
        "   background-color: #2d2d2d; "
        "   color: #ffffff; "
        "   padding: 8px; "
        "   border: 1px solid #3a3a3a; "
        "   font-weight: bold; "
        "}"
    );
    mainLayout->addWidget(leakTable_);

    // ===== CONTENEDOR DE GRÁFICAS CON SPLITTER =====
    QSplitter* chartsSplitter = new QSplitter(Qt::Horizontal, this);

    // Gráfica de barras
    barChartView_ = new QChartView(this);
    barChartView_->setRenderHint(QPainter::Antialiasing);
    barChartView_->setMinimumHeight(300);

    // Gráfica de pie
    pieChartView_ = new QChartView(this);
    pieChartView_->setRenderHint(QPainter::Antialiasing);
    pieChartView_->setMinimumHeight(300);

    chartsSplitter->addWidget(barChartView_);
    chartsSplitter->addWidget(pieChartView_);
    chartsSplitter->setStretchFactor(0, 1);
    chartsSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(chartsSplitter);

    // ===== GRÁFICA TEMPORAL =====
    timeChartView_ = new QChartView(this);
    timeChartView_->setRenderHint(QPainter::Antialiasing);
    timeChartView_->setMinimumHeight(250);
    mainLayout->addWidget(timeChartView_);

    // Proporciones del layout
    mainLayout->setStretch(0, 1);  // Resumen
    mainLayout->setStretch(1, 3);  // Tabla
    mainLayout->setStretch(2, 2);  // Barras + Pie
    mainLayout->setStretch(3, 2);  // Temporal
}

void MemoryLeaksTab::updateFromJson(const QString& json) {
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QJsonObject payload = root.value("payload").toObject();
    QJsonArray blocks = payload.value("blocks").toArray();

    LeakSummary summary;

    for (const QJsonValue& val : blocks) {
        QJsonObject block = val.toObject();

        LeakInfo info;
        info.file = block.value("file").toString();
        info.line = block.value("line").toInt();
        info.size = static_cast<size_t>(block.value("size").toDouble());
        info.type = block.value("type_name").toString();
        info.timestamp_ns = static_cast<uint64_t>(block.value("t_ns").toDouble());

        summary.total_leaked_bytes += info.size;
        summary.total_leaks += 1;
        summary.leaks.push_back(info);

        if (info.size > summary.largest_leak_size) {
            summary.largest_leak_size = info.size;
            summary.largest_leak_file = info.file;
        }

        summary.leakCountByFile[info.file] += 1;
        summary.leakBytesByFile[info.file] += info.size;
    }

    updateFromLeaks(summary);
}

void MemoryLeaksTab::updateFromLeaks(const LeakSummary& summary) {
    // ===== RESUMEN MEJORADO CON FORMATO =====
    QString text = QString(
    "\n"
        "RESUMEN DE MEMORY LEAKS\n"
        "Total fugado:      %1 KB (%2 MB)\n"
        "Total de leaks:    %3\n"
        "Archivo crítico:   %4\n"
        "Leak más grande:   %5 KB\n"
    )
        .arg(summary.total_leaked_bytes / 1024.0, 10, 'f', 2)
        .arg(summary.total_leaked_bytes / (1024.0 * 1024.0), 8, 'f', 2)
        .arg(summary.total_leaks, 10)
        .arg(summary.largest_leak_file.left(30), -30)
        .arg(summary.largest_leak_size / 1024.0, 10, 'f', 2);

    summaryLabel_->setText(text);

    // ===== TABLA CON TIMESTAMP LEGIBLE =====
    leakTable_->setRowCount(static_cast<int>(summary.leaks.size()));
    for (int i = 0; i < summary.leaks.size(); ++i) {
        const auto& leak = summary.leaks[i];

        leakTable_->setItem(i, 0, new QTableWidgetItem(leak.file));
        leakTable_->setItem(i, 1, new QTableWidgetItem(QString::number(leak.line)));

        // Formato de tamaño con color según severidad
        auto* sizeItem = new QTableWidgetItem(QString::number(leak.size / 1024.0, 'f', 2));
        if (leak.size > 100 * 1024) {  // > 100KB
            sizeItem->setForeground(QColor(255, 100, 100));  // Rojo
        } else if (leak.size > 10 * 1024) {  // > 10KB
            sizeItem->setForeground(QColor(255, 200, 100));  // Naranja
        }
        leakTable_->setItem(i, 2, sizeItem);

        leakTable_->setItem(i, 3, new QTableWidgetItem(leak.type));

        // Timestamp en formato legible (relativo en ms)
        double timeMs = leak.timestamp_ns / 1e6;
        leakTable_->setItem(i, 4, new QTableWidgetItem(QString::number(timeMs, 'f', 1) + " ms"));
    }

    // ===== GRÁFICA DE BARRAS MEJORADA =====
    QBarSeries* barSeries = new QBarSeries();
    QBarSet* set = new QBarSet("Memoria fugada");
    set->setColor(QColor(100, 180, 255));  // Azul

    QStringList categories;
    // Ordenar por tamaño y tomar top 10
    QList<QPair<QString, size_t>> fileList;
    for (auto it = summary.leakBytesByFile.begin(); it != summary.leakBytesByFile.end(); ++it) {
        fileList.append(qMakePair(it.key(), it.value()));
    }
    std::sort(fileList.begin(), fileList.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    int maxFiles = qMin(10, fileList.size());
    for (int i = 0; i < maxFiles; ++i) {
        QString fileName = fileList[i].first;
        // Extraer solo el nombre del archivo, no la ruta completa
        int lastSlash = fileName.lastIndexOf('/');
        if (lastSlash != -1) fileName = fileName.mid(lastSlash + 1);

        categories << fileName;
        *set << static_cast<qreal>(fileList[i].second) / 1024.0;
    }
    barSeries->append(set);

    QChart* barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("🔝 Top 10 Archivos con más Fugas (KB)");
    barChart->setAnimationOptions(QChart::SeriesAnimations);
    barChart->setTheme(QChart::ChartThemeDark);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    barChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Memoria (KB)");
    barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    barChart->legend()->setVisible(true);
    barChart->legend()->setAlignment(Qt::AlignBottom);

    barChartView_->setChart(barChart);

    // ===== GRÁFICA DE PIE MEJORADA =====
    QPieSeries* pieSeries = new QPieSeries();

    // Agrupar archivos pequeños en "Otros"
    size_t othersTotal = 0;
    for (int i = 0; i < maxFiles && i < 8; ++i) {
        QString fileName = fileList[i].first;
        int lastSlash = fileName.lastIndexOf('/');
        if (lastSlash != -1) fileName = fileName.mid(lastSlash + 1);

        QPieSlice* slice = pieSeries->append(fileName, static_cast<qreal>(fileList[i].second));
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);

        // Colores degradados
        int hue = (i * 360) / 8;
        slice->setColor(QColor::fromHsv(hue, 200, 230));
    }

    // Agrupar resto en "Otros"
    for (int i = 8; i < fileList.size(); ++i) {
        othersTotal += fileList[i].second;
    }
    if (othersTotal > 0) {
        QPieSlice* othersSlice = pieSeries->append("Otros", static_cast<qreal>(othersTotal));
        othersSlice->setColor(QColor(150, 150, 150));
        othersSlice->setLabelVisible(true);
    }

    QChart* pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("📊 Distribución de Fugas por Archivo");
    pieChart->setTheme(QChart::ChartThemeDark);
    pieChart->legend()->setVisible(true);
    pieChart->legend()->setAlignment(Qt::AlignRight);

    pieChartView_->setChart(pieChart);

    // ===== GRÁFICA TEMPORAL MEJORADA =====
    QScatterSeries* scatterSeries = new QScatterSeries();
    scatterSeries->setName("Detección de leaks");
    scatterSeries->setMarkerSize(8.0);
    scatterSeries->setColor(QColor(255, 100, 100));

    // Usar timestamp relativo (en milisegundos desde el primer leak)
    uint64_t minTime = summary.leaks.empty() ? 0 : summary.leaks[0].timestamp_ns;

    for (const auto& leak : summary.leaks) {
        qint64 relativeTime = static_cast<qint64>((leak.timestamp_ns - minTime) / 1e6);  // ms
        qreal sizeKB = static_cast<qreal>(leak.size) / 1024.0;
        scatterSeries->append(relativeTime, sizeKB);
    }

    QChart* timeChart = new QChart();
    timeChart->addSeries(scatterSeries);
    timeChart->setTitle("⏱️ Detección de Fugas en el Tiempo");
    timeChart->setTheme(QChart::ChartThemeDark);

    QValueAxis* axisXTime = new QValueAxis();
    axisXTime->setTitleText("Tiempo transcurrido (ms)");
    axisXTime->setLabelFormat("%d");
    timeChart->addAxis(axisXTime, Qt::AlignBottom);
    scatterSeries->attachAxis(axisXTime);

    QValueAxis* axisYTime = new QValueAxis();
    axisYTime->setTitleText("Tamaño del leak (KB)");
    timeChart->addAxis(axisYTime, Qt::AlignLeft);
    scatterSeries->attachAxis(axisYTime);

    timeChart->legend()->setVisible(true);
    timeChart->legend()->setAlignment(Qt::AlignBottom);

    timeChartView_->setChart(timeChart);
}