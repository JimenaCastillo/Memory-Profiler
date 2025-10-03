#include "../include/ProfilerController.hpp"
#include "../Library/include/ProfilerAPI.hpp"
#include "../Library/include/ProfilerNew.hpp"
#include <QHostAddress>
#include <QTcpSocket>

ProfilerController::ProfilerController(QObject* parent)
    : QObject(parent),
      timer_(MP_NEW_FT(QTimer, this)),
      server_(MP_NEW_FT(QTcpServer, this)),
      clientSocket_(nullptr)
{
    connect(server_, &QTcpServer::newConnection, this, &ProfilerController::onNewConnection);
}

ProfilerController::~ProfilerController() {
    stop();
}

void ProfilerController::start() {
    server_->listen(QHostAddress::LocalHost, 7777);  // GUI actúa como servidor
    timer_->start(200);  // Eitir métricas locales cada 200 ms
}

void ProfilerController::stop() {
    timer_->stop();
    server_->close();
    if (clientSocket_) {
        clientSocket_->disconnect();
        clientSocket_->close();
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
}

void ProfilerController::onNewConnection() {
    if (clientSocket_) {
        clientSocket_->disconnect();
        clientSocket_->close();
        clientSocket_->deleteLater();
    }
    clientSocket_ = server_->nextPendingConnection();
    connect(clientSocket_, &QTcpSocket::readyRead, this, &ProfilerController::onReadyRead);

    emit clientConnected();
}

void ProfilerController::onReadyRead() {
    QByteArray data = clientSocket_->readAll();
    emit metricsUpdated(QString::fromUtf8(data));
}

QString ProfilerController::getLatestMetrics() const {
    return QString::fromStdString(mp::api::getMetricsJson());
}

QString ProfilerController::getSnapshot() const {
    return QString::fromStdString(mp::api::getSnapshotJson());
}

bool ProfilerController::hasClientConnected() const {
    return clientSocket_ && clientSocket_->state() == QAbstractSocket::ConnectedState;
}