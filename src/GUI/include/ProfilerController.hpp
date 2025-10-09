#pragma once
#include <QObject>
#include <QTimer>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>

class ProfilerController : public QObject {
    Q_OBJECT
public:
    explicit ProfilerController(QObject* parent = nullptr);
    ~ProfilerController();

    void start();
    void stop();

    [[nodiscard]] QString getLatestMetrics() const;
    [[nodiscard]] QString getSnapshot() const;
    [[nodiscard]] bool hasClientConnected() const;

    void requestSnapshot();

    signals:
        void metricsUpdated(const QString& json);
    void clientConnected();

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QTimer* timer_;
    QTcpServer* server_;
    QTcpSocket* clientSocket_;
    QString receiveBuffer_;
};