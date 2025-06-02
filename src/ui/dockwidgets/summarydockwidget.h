#ifndef SummaryDockWidget_H
#define SummaryDockWidget_H

#include <QDockwidget>

class SummaryDockWidget : public QDockwidget {
    Q_OBJECT
public:
    explicit SummaryDockWidget(QObject* = nullptr);
    SummaryDockWidget();
};

#endif