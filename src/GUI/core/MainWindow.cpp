#include "../include/MainWindow.hpp"
#include "../include/ProfilerController.hpp"
#include "../include/MemoryChart.hpp"
#include "../include/MemoryMapView.hpp"
#include "../Library/include/ProfilerNew.hpp"
#include "../include/FileAllocationStats.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      controller_(MP_NEW_FT(ProfilerController, this)),
      statusLabel_(MP_NEW_FT(QLabel,"Estado: detenido")),
      metricsView_( MP_NEW_FT(QTextEdit)),
      startButton_(MP_NEW_FT(QPushButton,"Iniciar")),
      stopButton_(MP_NEW_FT(QPushButton,"Detener")),
      snapshotButton_(MP_NEW_FT(QPushButton,"Snapshot"))
{
    metricsView_->setReadOnly(true);

    tabWidget_ = MP_NEW_FT(QTabWidget, this);

    // Vista general
    generalTab_ = MP_NEW_FT(QWidget);
    auto* generalLayout = MP_NEW_FT(QVBoxLayout, generalTab_);
    generalLayout->addWidget(statusLabel_);
    generalLayout->addWidget(metricsView_);

    chartView_ = MP_NEW_FT(MemoryChart);
    generalLayout->addWidget(chartView_);

    topAllocationsTable_ = MP_NEW_FT(QTableWidget, this);
    topAllocationsTable_->setColumnCount(3);
    topAllocationsTable_->setHorizontalHeaderLabels({"Archivo", "Asignaciones", "Memoria"});
    topAllocationsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    generalLayout->addWidget(topAllocationsTable_);

    auto* buttonLayout = MP_NEW_FT(QVBoxLayout);
    buttonLayout->addWidget(startButton_);
    buttonLayout->addWidget(stopButton_);
    buttonLayout->addWidget(snapshotButton_);
    generalLayout->addLayout(buttonLayout);

    tabWidget_->addTab(generalTab_, "Vista general");

    // Mapa de memoria
    memoryMapTab_ = MP_NEW_FT(QWidget);
    memoryMapView_ = MP_NEW_FT(MemoryMapView);
    auto* mapLayout = MP_NEW_FT(QVBoxLayout, memoryMapTab_);
    mapLayout->addWidget(memoryMapView_);
    tabWidget_->addTab(memoryMapTab_, "Mapa de memoria");

    // Asignación por archivo fuente
    fileAllocTabContainer_ = MP_NEW_FT(QWidget);
    fileAllocTab_ = MP_NEW_FT(FileAllocationsTab);
    auto* fileLayout = MP_NEW_FT(QVBoxLayout, fileAllocTabContainer_);
    fileLayout->addWidget(fileAllocTab_);
    tabWidget_->addTab(fileAllocTabContainer_, "Asignación por archivo");


    // Finaliza
    setCentralWidget(tabWidget_);

    setWindowTitle("Memory Profiler");

    statusBar_ = MP_NEW_FT(QStatusBar,this);
    setStatusBar(statusBar_);
    statusBar_->showMessage("Listo para iniciar el profiling");

    connect(startButton_, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(stopButton_, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(snapshotButton_, &QPushButton::clicked, this, &MainWindow::onSnapshotClicked);
    connect(controller_, &ProfilerController::metricsUpdated, this, &MainWindow::updateMetrics);
}

void MainWindow::onStartClicked() {
    controller_->start();
    if (controller_->isRunning())
        statusLabel_->setText("Estado: ejecutando");
    else
        statusLabel_->setText("Estado: desconectado");
    statusBar_->showMessage("Profiling iniciado", 3000);  // mensaje por 3 segundos
}

void MainWindow::onStopClicked() {
    controller_->stop();
    statusLabel_->setText("Estado: detenido");
    statusBar_->showMessage("Profiling detenido", 3000);
}

void MainWindow::onSnapshotClicked() {
    QString snapshot = controller_->getSnapshot();
    metricsView_->append("📸 Snapshot:\n" + snapshot + "\n");
    statusBar_->showMessage("📸 Snapshot capturado", 3000);
    memoryMapView_->updateFromJson(snapshot);
    updateMetrics(snapshot);
}

void MainWindow::updateMetrics(const QString& json) {
    metricsView_->setPlainText("📊 Métricas:\n" + json);
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isObject()) {
        QJsonObject root = doc.object();
        QJsonObject payload = root.value("payload").toObject();
        double mem = payload.value("bytes_in_use").toDouble();  // ajusta según tu JSON
        chartView_->addDataPoint(mem/1024.0);

        auto topFiles = computeTopAllocFiles();
        topAllocationsTable_->setRowCount(static_cast<int>(topFiles.size()));

        for (int i = 0; i < topFiles.size(); ++i) {
            const auto& s = topFiles[i];
            topAllocationsTable_->setItem(i, 0, new QTableWidgetItem(s.file));
            topAllocationsTable_->setItem(i, 1, new QTableWidgetItem(QString::number(s.count)));
            topAllocationsTable_->setItem(i, 2, new QTableWidgetItem(QString::number(s.total_bytes / 1024.0, 'f', 2) + " KB"));
        }
    }
}