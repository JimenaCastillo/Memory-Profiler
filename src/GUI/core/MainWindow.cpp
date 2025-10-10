#include "../include/MainWindow.hpp"
#include "../include/ProfilerController.hpp"
#include "../include/MemoryChart.hpp"
#include "../include/MemoryMapView.hpp"
#include "../Library/include/ProfilerNew.hpp"
#include "../include/FileAllocationStats.hpp"
#include "include/LeakStats.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    controller_ = new ProfilerController(this);
    statusLabel_ = new QLabel("Estado: escuchando");
    metricsView_ = new QTextEdit();
    stopButton_ = new QPushButton("Detener");
    snapshotButton_ = new QPushButton("Snapshot");

    metricsView_->setReadOnly(true);
    tabWidget_ = new QTabWidget(this);

    // Vista general
    generalTab_ = new QWidget();
    auto* generalLayout = new QVBoxLayout(generalTab_);
    generalLayout->addWidget(statusLabel_);
    generalLayout->addWidget(metricsView_);

    chartView_ = new MemoryChart();
    generalLayout->addWidget(chartView_);

    topAllocationsTable_ = new QTableWidget(this);
    topAllocationsTable_->setColumnCount(3);
    topAllocationsTable_->setHorizontalHeaderLabels({"Archivo", "Asignaciones", "Memoria"});
    topAllocationsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    generalLayout->addWidget(topAllocationsTable_);

    auto* buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(stopButton_);
    buttonLayout->addWidget(snapshotButton_);
    generalLayout->addLayout(buttonLayout);

    tabWidget_->addTab(generalTab_, "Vista general");

    // Mapa de memoria
    memoryMapTab_ = new QWidget();
    memoryMapView_ = new MemoryMapView();
    auto* mapLayout = new QVBoxLayout(memoryMapTab_);
    mapLayout->addWidget(memoryMapView_);
    tabWidget_->addTab(memoryMapTab_, "Mapa de memoria");

    // Asignación por archivo fuente
    fileAllocTabContainer_ = new QWidget();
    fileAllocTab_ = new FileAllocationsTab();
    auto* fileLayout = new QVBoxLayout(fileAllocTabContainer_);
    fileLayout->addWidget(fileAllocTab_);
    tabWidget_->addTab(fileAllocTabContainer_, "Asignación por archivo");

    // Memory Leaks
    leaksTabContainer_ = new QWidget();
    leaksTab_ = new MemoryLeaksTab();
    auto* leaksLayout = new QVBoxLayout(leaksTabContainer_);
    leaksLayout->addWidget(leaksTab_);
    tabWidget_->addTab(leaksTabContainer_, "Memory leaks");

    // Finaliza
    setCentralWidget(tabWidget_);
    setWindowTitle("Memory Profiler");

    statusBar_ = new QStatusBar(this);
    setStatusBar(statusBar_);
    statusBar_->showMessage("Servidor activo en puerto 7777");

    connect(stopButton_, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(snapshotButton_, &QPushButton::clicked, this, &MainWindow::onSnapshotClicked);
    connect(controller_, &ProfilerController::metricsUpdated, this, &MainWindow::updateMetrics);
    connect(controller_, &ProfilerController::clientConnected, this, [this]() {
        statusLabel_->setText("Estado: conectado");
        statusBar_->showMessage("Cliente conectado", 3000);
    });

    controller_->start();
}

void MainWindow::onStopClicked() {
    if (statusLabel_->text() == "Estado: escuchando") {
        controller_->stop();
        statusLabel_->setText("Estado: detenido");
        statusBar_->showMessage("Servidor detenido", 3000);
        stopButton_->setText("Reanudar");
    } else {
        controller_->start();
        statusLabel_->setText("Estado: escuchando");
        statusBar_->showMessage("Servidor reanudado", 3000);
        stopButton_->setText("Detener");
    }
}

void MainWindow::onSnapshotClicked() {
    if (!controller_->hasClientConnected()) {
        statusBar_->showMessage("No hay cliente conectado. Snapshot no disponible.", 3000);
        return;
    }

    controller_->requestSnapshot();
    statusBar_->showMessage("📸 Solicitando snapshot...", 3000);
}

void MainWindow::updateMetrics(const QString& json) {
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QString type = root.value("type").toString();

    if (type == "SUMMARY") {
        metricsView_->setPlainText("📊 Métricas:\n" + json);

        QJsonObject payload = root.value("payload").toObject();
        double mem = payload.value("bytes_in_use").toDouble();
        chartView_->addDataPoint(mem / 1024.0);

    } else if (type == "LIVE_ALLOCS") {
        metricsView_->append("\n📸 Snapshot recibido:\n" + json);
        statusBar_->showMessage("📸 Snapshot actualizado", 3000);

        QJsonObject payload = root.value("payload").toObject();
        QJsonArray blocks = payload.value("blocks").toArray();

        // Actualizar todas las vistas
        memoryMapView_->updateFromJson(json);
        fileAllocTab_->updateFromJson(json);
        leaksTab_->updateFromJson(json);

        // Actualizar top allocations - FILTRAR entradas sin archivo
        QMap<QString, FileAllocStats> fileStats;

        for (const QJsonValue& val : blocks) {
            QJsonObject block = val.toObject();
            QString file = block.value("file").toString();
            int line = block.value("line").toInt();
            double size = block.value("size").toDouble();

            // FILTRAR: Ignorar bloques sin información de archivo válida
            if (file.isEmpty() || file == "?" || line == 0) {
                continue;
            }

            QString key = file + ":" + QString::number(line);
            fileStats[key].file = key;
            fileStats[key].count += 1;
            fileStats[key].total_bytes += static_cast<size_t>(size);
        }

        // Ordenar por tamaño y tomar top 3
        QList<FileAllocStats> statsList = fileStats.values();
        std::sort(statsList.begin(), statsList.end(), [](const FileAllocStats& a, const FileAllocStats& b) {
            return a.total_bytes > b.total_bytes;
        });

        int topN = std::min(3, static_cast<int>(statsList.size()));
        topAllocationsTable_->setRowCount(topN);

        for (int i = 0; i < topN; ++i) {
            const auto& s = statsList[i];
            topAllocationsTable_->setItem(i, 0, new QTableWidgetItem(s.file));
            topAllocationsTable_->setItem(i, 1, new QTableWidgetItem(QString::number(s.count)));
            topAllocationsTable_->setItem(i, 2, new QTableWidgetItem(QString::number(s.total_bytes / 1024.0, 'f', 2) + " KB"));
        }
    }
}