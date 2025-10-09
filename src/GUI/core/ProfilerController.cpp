#include "../include/ProfilerController.hpp"
#include "../Library/include/ProfilerAPI.hpp"
#include "../Library/include/ProfilerNew.hpp"
#include <QHostAddress>
#include <QTcpSocket>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

ProfilerController::ProfilerController(QObject* parent)
    : QObject(parent),
      timer_(MP_NEW_FT(QTimer, this)),
      server_(MP_NEW_FT(QTcpServer, this)),
      clientSocket_(nullptr),
      receiveBuffer_()
{
    connect(server_, &QTcpServer::newConnection, this, &ProfilerController::onNewConnection);
}

ProfilerController::~ProfilerController() {
    stop();
}

void ProfilerController::start() {
    if (server_->listen(QHostAddress::LocalHost, 7777)) {
        qDebug() << "[ProfilerController] Servidor escuchando en puerto 7777";
    } else {
        qDebug() << "[ProfilerController] Error al iniciar servidor:" << server_->errorString();
    }
    timer_->start(200);
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
    qDebug() << "[ProfilerController] Nueva conexión recibida";

    if (clientSocket_) {
        clientSocket_->disconnect();
        clientSocket_->close();
        clientSocket_->deleteLater();
    }
    clientSocket_ = server_->nextPendingConnection();
    receiveBuffer_.clear();

    connect(clientSocket_, &QTcpSocket::readyRead, this, &ProfilerController::onReadyRead);
    emit clientConnected();
}

// Función auxiliar para verificar si tenemos un JSON completo
static bool isCompleteJson(const QString& str) {
    int braceCount = 0;
    bool inString = false;
    bool escape = false;

    for (int i = 0; i < str.length(); ++i) {
        QChar c = str[i];

        if (escape) {
            escape = false;
            continue;
        }

        if (c == '\\') {
            escape = true;
            continue;
        }

        if (c == '"') {
            inString = !inString;
            continue;
        }

        if (inString) {
            continue;
        }

        if (c == '{') {
            braceCount++;
        } else if (c == '}') {
            braceCount--;
            if (braceCount == 0) {
                // JSON completo encontrado
                return true;
            }
        }
    }

    return false;
}

void ProfilerController::onReadyRead() {
    QByteArray data = clientSocket_->readAll();
    receiveBuffer_.append(QString::fromUtf8(data));

    qDebug() << "[ProfilerController] Buffer acumulado:" << receiveBuffer_.size() << "bytes";

    // Intentar extraer mensajes JSON completos
    while (!receiveBuffer_.isEmpty()) {
        // Buscar el inicio de un JSON
        int jsonStart = receiveBuffer_.indexOf('{');
        if (jsonStart == -1) {
            // No hay JSON en el buffer
            receiveBuffer_.clear();
            break;
        }

        // Descartar contenido antes del JSON
        if (jsonStart > 0) {
            receiveBuffer_.remove(0, jsonStart);
        }

        // Verificar si tenemos un JSON completo
        if (!isCompleteJson(receiveBuffer_)) {
            // JSON incompleto, esperar más datos
            qDebug() << "[ProfilerController] JSON incompleto, esperando más datos...";
            break;
        }

        // Encontrar el final del JSON completo
        int braceCount = 0;
        bool inString = false;
        bool escape = false;
        int jsonEnd = -1;

        for (int i = 0; i < receiveBuffer_.length(); ++i) {
            QChar c = receiveBuffer_[i];

            if (escape) {
                escape = false;
                continue;
            }

            if (c == '\\') {
                escape = true;
                continue;
            }

            if (c == '"') {
                inString = !inString;
                continue;
            }

            if (inString) {
                continue;
            }

            if (c == '{') {
                braceCount++;
            } else if (c == '}') {
                braceCount--;
                if (braceCount == 0) {
                    jsonEnd = i;
                    break;
                }
            }
        }

        if (jsonEnd == -1) {
            // No deberíamos llegar aquí, pero por seguridad
            break;
        }

        // Extraer el mensaje JSON completo
        QString message = receiveBuffer_.left(jsonEnd + 1);
        receiveBuffer_.remove(0, jsonEnd + 1);

        // Remover el \n si existe después del JSON
        if (!receiveBuffer_.isEmpty() && receiveBuffer_[0] == '\n') {
            receiveBuffer_.remove(0, 1);
        }

        qDebug() << "[ProfilerController] JSON completo extraído, tamaño:" << message.size() << "bytes";

        // Validar JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[ProfilerController] Error parseando JSON:" << parseError.errorString();
            qDebug() << "[ProfilerController] Offset del error:" << parseError.offset;
            qDebug() << "[ProfilerController] Preview:" << message.left(200);
            continue;
        }

        qDebug() << "[ProfilerController] ✓ JSON válido recibido";

        // Identificar tipo de mensaje
        QJsonObject root = doc.object();
        QString type = root.value("type").toString();
        qDebug() << "[ProfilerController] Tipo de mensaje:" << type;

        emit metricsUpdated(message);
    }

    // Protección: limpiar buffer si crece demasiado
    if (receiveBuffer_.size() > 50 * 1024 * 1024) {  // 50 MB
        qDebug() << "[ProfilerController] WARNING: Buffer muy grande (" << receiveBuffer_.size()
                 << "bytes), limpiando...";
        receiveBuffer_.clear();
    }
}

void ProfilerController::requestSnapshot() {
    if (clientSocket_ && clientSocket_->state() == QAbstractSocket::ConnectedState) {
        QString command = "SNAPSHOT\n";
        qDebug() << "[ProfilerController] Enviando comando SNAPSHOT al cliente";

        qint64 written = clientSocket_->write(command.toUtf8());
        clientSocket_->flush();

        qDebug() << "[ProfilerController] Bytes escritos:" << written;
    } else {
        qDebug() << "[ProfilerController] No hay cliente conectado";
    }
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