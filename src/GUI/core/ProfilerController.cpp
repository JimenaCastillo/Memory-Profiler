#include "../include/ProfilerController.hpp"
#include "../Library/include/ProfilerAPI.hpp"
#include <QHostAddress>
#include <QTcpSocket>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

ProfilerController::ProfilerController(QObject* parent)
    : QObject(parent),
      timer_(new QTimer(this)),
      server_(new QTcpServer(this)),
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

    while (!receiveBuffer_.isEmpty()) {
        // 1. Encontrar inicio de JSON
        int jsonStart = receiveBuffer_.indexOf('{');
        if (jsonStart == -1) {
            receiveBuffer_.clear();
            break;
        }

        if (jsonStart > 0) {
            receiveBuffer_.remove(0, jsonStart);
        }

        // 2. Verificar si tenemos un JSON completo
        if (!isCompleteJson(receiveBuffer_)) {
            qDebug() << "[ProfilerController] JSON incompleto, esperando más datos...";
            break; // Esperar más datos
        }

        //3. Extraer el JSON completo
        int braceCount = 0;
        bool inString = false;
        bool escape = false;
        int jsonEnd = -1;

        for (int i = 0; i < receiveBuffer_.length(); ++i) {
            QChar c = receiveBuffer_[i];

            // Manejo de strings
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
                    break; // JSON completo encontrado
                }
            }
        }

        if (jsonEnd == -1) {
            break;
        }

        // 4. Extraer y procesar mensaje
        QString message = receiveBuffer_.left(jsonEnd + 1);
        receiveBuffer_.remove(0, jsonEnd + 1);

        if (!receiveBuffer_.isEmpty() && receiveBuffer_[0] == '\n') {
            receiveBuffer_.remove(0, 1);
        }

        qDebug() << "[ProfilerController] JSON completo extraído, tamaño:" << message.size() << "bytes";

        // 5. Parsear y emitir señal
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[ProfilerController] Error parseando JSON:" << parseError.errorString();
            qDebug() << "[ProfilerController] Offset del error:" << parseError.offset;
            qDebug() << "[ProfilerController] Preview:" << message.left(200);
            continue;
        }

        qDebug() << "[ProfilerController] ✓ JSON válido recibido";

        QJsonObject root = doc.object();
        QString type = root.value("type").toString();
        qDebug() << "[ProfilerController] Tipo de mensaje:" << type;

        emit metricsUpdated(message);
    }

    if (receiveBuffer_.size() > 50 * 1024 * 1024) {
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