#include "../include/ProfilerController.hpp"
#include "ProfilerAPI.hpp"  // mp::api::{getMetricsJson, getSnapshotJson}

ProfilerController::ProfilerController(QObject* parent)
    : QObject(parent),
      timer_(new QTimer(this))
{
    // Actualiza métricas cada 200 ms
    connect(timer_, &QTimer::timeout, this, [this]() {
        QString json = QString::fromStdString(mp::api::getMetricsJson());
        emit metricsUpdated(json);
    });
}

ProfilerController::~ProfilerController() {
    stop();  // Asegura que el hilo se detenga correctamente
}

void ProfilerController::start() {
    if (!client_.isRunning()) {
        client_.start();  // Usa host y puerto por defecto (127.0.0.1:7777)
        timer_->start(200);  // Intervalo de actualización
    }
}

void ProfilerController::stop() {
    timer_->stop();
    client_.stop();
}

bool ProfilerController::isRunning() const {
    return client_.isRunning();
}

QString ProfilerController::getLatestMetrics() const {
    return QString::fromStdString(mp::api::getMetricsJson());
}

QString ProfilerController::getSnapshot() const {
    return QString::fromStdString(mp::api::getSnapshotJson());
}

