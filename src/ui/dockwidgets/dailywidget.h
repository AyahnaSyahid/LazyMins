#ifndef DailyWidget_H
#define DailyWidget_H

namespace Ui {
    class DailyWidget;
}

#include <QWidget>
#include <QDockWidget>

class Database;
class DailyWidget : public QWidget {
    Q_OBJECT

public:
    explicit DailyWidget(Database*, QWidget* =nullptr);
    ~DailyWidget();
    
public slots:
    void reloadData();

private:
    Ui::DailyWidget* ui;
    Database* db;
};

class DailyDockWidget : public QDockWidget {
public:
    explicit DailyDockWidget(Database*, QWidget* =nullptr);
    ~DailyDockWidget();
};

#endif