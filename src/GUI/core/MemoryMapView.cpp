#include "../include/MemoryMapView.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGraphicsRectItem>
#include <QToolTip>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>

#include "ProfilerNew.hpp"

MemoryMapView::MemoryMapView(QWidget* parent)
    : QGraphicsView(parent), scene_(MP_NEW_FT(QGraphicsScene, this)) {
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

    qreal x = 0;
    qreal y = 0;
    qreal maxHeight = 100;

    for (const QJsonValue& val : blocks) {
        QJsonObject obj = val.toObject();
        qreal size = obj.value("size").toDouble();
        QString tooltip = QString("📦 ptr: %1\nsize: %2\nthread: %3\ncallsite: %4")
            .arg(obj.value("ptr").toString())
            .arg(size)
            .arg(obj.value("thread_id").toInt())
            .arg(obj.value("callsite").toString());

        qreal width = size / 1024.0;  // escala visual
        QGraphicsRectItem* rect = scene_->addRect(x, y, width, maxHeight);
        rect->setToolTip(tooltip);
        rect->setBrush(Qt::cyan);
        x += width + 5;
        if (x > 800) { x = 0; y += maxHeight + 10; }
    }

    scene_->setSceneRect(0, 0, 1000, y + maxHeight);
}