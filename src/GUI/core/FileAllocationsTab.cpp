#include "../include/FileAllocationsTab.hpp"
#include "../include/FileAllocationStats.hpp"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>

FileAllocationsTab::FileAllocationsTab(QWidget* parent)
    : QWidget(parent),
      table_(new QTableWidget(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(table_);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({"Archivo", "Asignaciones", "Memoria total"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    refresh();
}

void FileAllocationsTab::refresh() {
    auto stats = computeFileAllocStats();
    table_->setRowCount(static_cast<int>(stats.size()));

    for (int i = 0; i < stats.size(); ++i) {
        const auto& s = stats[i];
        table_->setItem(i, 0, new QTableWidgetItem(s.file));
        table_->setItem(i, 1, new QTableWidgetItem(QString::number(s.count)));
        table_->setItem(i, 2, new QTableWidgetItem(QString::number(s.total_bytes / 1024.0, 'f', 2) + " KB"));
    }
}
