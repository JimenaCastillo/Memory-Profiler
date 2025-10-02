#pragma once
#include <QWidget>
class QTableWidget;

class FileAllocationsTab : public QWidget {
    Q_OBJECT
public:
    explicit FileAllocationsTab(QWidget* parent = nullptr);
    void refresh();
private:
    QTableWidget* table_;
};