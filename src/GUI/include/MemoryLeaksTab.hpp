#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSlice>
#include "LeakStats.hpp"

class MemoryLeaksTab : public QWidget {
    Q_OBJECT
public:
    explicit MemoryLeaksTab(QWidget* parent = nullptr);
    void updateFromJson(const QString& json);
    void updateFromLeaks(const LeakSummary& summary);

private:
    QLabel* summaryLabel_;
    QTableWidget* leakTable_;
    QChartView* barChartView_;
    QChartView* pieChartView_;
    QChartView* timeChartView_;
};