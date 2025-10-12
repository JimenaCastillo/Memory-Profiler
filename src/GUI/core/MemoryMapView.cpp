#include "../include/MemoryMapView.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGraphicsRectItem>
#include <QToolTip>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QDebug>

MemoryMapView::MemoryMapView(QWidget* parent)
    : QGraphicsView(parent), scene_(new QGraphicsScene(this)) {
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing);
}

void MemoryMapView::updateFromJson(const QString& json) {
    scene_->clear();

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QJsonObject payload = root.value("payload").toObject();
    QJsonArray blocks = payload.value("blocks").toArray();

    qDebug() << "[MemoryMapView] Procesando" << blocks.size() << "bloques";

    qreal x = 0;
    qreal y = 0;
    qreal maxHeight = 100;

    int validBlocks = 0;
    int unknownBlocks = 0;

    for (const QJsonValue& val : blocks) {
        QJsonObject obj = val.toObject();
        qreal size = obj.value("size").toDouble();
        QString file = obj.value("file").toString();
        int line = obj.value("line").toInt();
        QString typeName = obj.value("type_name").toString();

        // Determinar si tiene información válida
        bool hasValidInfo = (!file.isEmpty() && file != "?" && line != 0);

        if (hasValidInfo) {
            validBlocks++;
            qDebug() << "[MemoryMapView] Bloque válido:" << file << ":" << line;
        } else {
            unknownBlocks++;
        }

        // Crear tooltip informativo
        QString callsiteInfo = hasValidInfo
            ? QString("%1:%2").arg(file).arg(line)
            : "desconocido";

        QString tooltip = QString("Información del bloque:\n"
                                 "├─ Dirección: %1\n"
                                 "├─ Tamaño: %2 bytes (%3 KB)\n"
                                 "├─ Tipo: %4\n"
                                 "├─ Thread ID: %5\n"
                                 "└─ Callsite: %6")
            .arg(obj.value("ptr").toString())
            .arg(size)
            .arg(size / 1024.0, 0, 'f', 2)
            .arg(typeName.isEmpty() ? "unknown" : typeName)
            .arg(obj.value("thread_id").toInt())
            .arg(callsiteInfo);

        // Calcular ancho visual (escala logarítmica para mejor visualización)
        qreal width = qMax(2.0, qMin(100.0, size / 512.0));

        QGraphicsRectItem* rect = scene_->addRect(x, y, width, maxHeight);
        rect->setToolTip(tooltip);

        // COLORES
        if (hasValidInfo) {
            // CYAN brillante para bloques con información
            rect->setBrush(QColor(0, 255, 255));  // Cyan puro
            rect->setPen(QPen(QColor(0, 200, 200), 1));
        } else {
            // ROJO/ROSA para bloques sin información
            rect->setBrush(QColor(255, 150, 150));
            rect->setPen(QPen(QColor(200, 100, 100), 1));
        }

        x += width + 2;
        if (x > 1200) {
            x = 0;
            y += maxHeight + 5;
        }
    }

    scene_->setSceneRect(0, 0, 1200, y + maxHeight + 20);

    qDebug() << "[MemoryMapView] Resumen:";
    qDebug() << "  - Total bloques:" << blocks.size();
    qDebug() << "  - Bloques VÁLIDOS (cyan):" << validBlocks;
    qDebug() << "  - Bloques DESCONOCIDOS (rojo):" << unknownBlocks;
}