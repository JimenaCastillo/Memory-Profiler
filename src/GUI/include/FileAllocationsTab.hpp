#pragma once
#include <QWidget>
#include <QString>
#include "FileAllocationStats.hpp"

class QTableWidget;

class FileAllocationsTab : public QWidget {
    Q_OBJECT
public:
    explicit FileAllocationsTab(QWidget* parent = nullptr);
    void updateFromJson(const QString& json);

private:
    QTableWidget* table_;
};