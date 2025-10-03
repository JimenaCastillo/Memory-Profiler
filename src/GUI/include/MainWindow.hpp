#pragma once
#include <QMainWindow>
#include "MemoryChart.hpp"
#include "MemoryMapView.hpp"
#include "FileAllocationsTab.hpp"
#include "MemoryLeaksTab.hpp"

class QLabel;
class QPushButton;
class QTextEdit;
class ProfilerController;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
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
    MemoryMapView* memoryMapView_;
    QTabWidget* tabWidget_;
    QWidget* generalTab_;
    QWidget* memoryMapTab_;
    FileAllocationsTab* fileAllocTab_;
    QWidget* fileAllocTabContainer_;
    QTableWidget* topAllocationsTable_;
    QWidget* leaksTabContainer_;
    MemoryLeaksTab* leaksTab_;
};