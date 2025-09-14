#pragma once
#include <QWidget>

namespace Tabs {
    QWidget* createOverviewTab();
    QWidget* createMemoryMapTab();
    QWidget* createByFileTab();
    QWidget* createLeaksTab();
}