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

    void start();       // Inicia el servidor y el temporizador
    void stop();        // Detiene el servidor y el temporizador

    [[nodiscard]] QString getLatestMetrics() const;
    [[nodiscard]] QString getSnapshot() const;
    [[nodiscard]] bool hasClientConnected() const;

    signals:
        void metricsUpdated(const QString& json);  // Señal con datos recibidos
        void clientConnected();

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QTimer* timer_;
    QTcpServer* server_;
    QTcpSocket* clientSocket_;
};