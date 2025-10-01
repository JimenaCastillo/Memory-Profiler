#pragma once
#include <QObject>
#include <QTimer>
#include <QString>
#include "SocketClient.hpp"

class ProfilerController : public QObject {
    Q_OBJECT
public:
    explicit ProfilerController(QObject* parent = nullptr);
    ~ProfilerController();

    // Inicia el profiling y comienza a emitir métricas periódicamente
    void start();

    // Detiene el profiling y la emisión de métricas
    void stop();

    // Verifica si el cliente está activo
    bool isRunning() const;

    // Obtiene las métricas actuales en formato JSON
    QString getLatestMetrics() const;

    // Solicita un snapshot en formato JSON
    QString getSnapshot() const;

    signals:
        // Señal emitida cada 200 ms con las métricas actualizadas
        void metricsUpdated(const QString& json);

private:
    QTimer* timer_;              // Temporizador para emitir métricas periódicas
    mp::SocketClient client_;    // Cliente TCP que se comunica con el servidor
};
