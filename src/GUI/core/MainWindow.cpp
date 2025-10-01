#include "../include/MainWindow.hpp"
#include "../include/ProfilerController.hpp"
#include "../include/MemoryChart.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStatusBar>



MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      controller_(new ProfilerController(this)),
      statusLabel_(new QLabel("Estado: detenido")),
      metricsView_(new QTextEdit),
      startButton_(new QPushButton("Iniciar")),
      stopButton_(new QPushButton("Detener")),
      snapshotButton_(new QPushButton("Snapshot"))
{
    metricsView_->setReadOnly(true);

    auto* layout = new QVBoxLayout;
    layout->addWidget(statusLabel_);
    layout->addWidget(metricsView_);

    chartView_ = new MemoryChart;
    layout->addWidget(chartView_);

    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton_);
    buttonLayout->addWidget(stopButton_);
    buttonLayout->addWidget(snapshotButton_);
    layout->addLayout(buttonLayout);

    auto* central = new QWidget;
    central->setLayout(layout);
    setCentralWidget(central);
    setWindowTitle("Memory Profiler");

    statusBar_ = new QStatusBar(this);
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
}

void MainWindow::updateMetrics(const QString& json) {
    metricsView_->setPlainText("📊 Métricas:\n" + json);
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        double mem = obj.value("active_bytes").toDouble();  // ajusta según tu JSON
        chartView_->addDataPoint(mem/1024.0);
    }
}
