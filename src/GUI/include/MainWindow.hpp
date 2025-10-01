#pragma once
#include <QMainWindow>
#include "MemoryChart.hpp"

class QLabel;
class QPushButton;
class QTextEdit;
class ProfilerController;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onSnapshotClicked();
    void updateMetrics(const QString& json);

private:
    ProfilerController* controller_;
    QLabel* statusLabel_;
    QTextEdit* metricsView_;
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QPushButton* snapshotButton_;
    MemoryChart* chartView_;
    QStatusBar* statusBar_;
};