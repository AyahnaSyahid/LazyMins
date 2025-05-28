#ifndef DashboardWidget_H
#define DashboardWidget_H

namespace Ui {
    class DashboardWidget;
}

#include <QWidget>

class Database;
class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    DashboardWidget(Database*, QWidget* =nullptr);
    ~DashboardWidget();
    
public slots:
    void reloadData();

private:
    Ui::DashboardWidget* ui;
    Database* db;
};

#endif