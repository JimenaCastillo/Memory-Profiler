#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QString>

class MemoryMapView : public QGraphicsView {
    Q_OBJECT
public:
    explicit MemoryMapView(QWidget* parent = nullptr);
    void updateFromJson(const QString& json);

private:
    QGraphicsScene* scene_;
};
