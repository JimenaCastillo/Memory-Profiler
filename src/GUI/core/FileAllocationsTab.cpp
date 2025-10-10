#include "../include/FileAllocationsTab.hpp"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

FileAllocationsTab::FileAllocationsTab(QWidget* parent)
    : QWidget(parent),
      table_(new QTableWidget(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(table_);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({"Archivo", "Asignaciones", "Memoria total"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void FileAllocationsTab::updateFromJson(const QString& json) {
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QJsonObject payload = root.value("payload").toObject();
    QJsonArray blocks = payload.value("blocks").toArray();

    QMap<QString, FileAllocStats> fileStats;

    for (const QJsonValue& val : blocks) {
        QJsonObject block = val.toObject();
        QString file = block.value("file").toString();
        int line = block.value("line").toInt();
        double size = block.value("size").toDouble();

        // FILTRAR bloques sin información válida
        if (file.isEmpty() || file == "?" || line == 0) {
            continue;
        }

        QString key = file + ":" + QString::number(line);
        fileStats[key].file = key;
        fileStats[key].count += 1;
        fileStats[key].total_bytes += static_cast<size_t>(size);
    }

    QList<FileAllocStats> statsList = fileStats.values();
    table_->setRowCount(statsList.size());

    for (int i = 0; i < statsList.size(); ++i) {
        const auto& s = statsList[i];
        table_->setItem(i, 0, new QTableWidgetItem(s.file));
        table_->setItem(i, 1, new QTableWidgetItem(QString::number(s.count)));
        table_->setItem(i, 2, new QTableWidgetItem(QString::number(s.total_bytes / 1024.0, 'f', 2) + " KB"));
    }
}