#ifndef CustomerOrdersWidget_H
#define CustomerOrdersWidget_H

#include <QWidget>
#include <QDockWidget>

namespace Ui {
    class CustomerOrdersWidget;
};

class Database;
class QSqlQueryModel;
class CustomerOrdersWidget : public QWidget {
    Q_OBJECT
public:
    explicit CustomerOrdersWidget(Database*, QWidget* =nullptr);
    ~CustomerOrdersWidget();
private slots:
    void reloadData();
private:
    Ui::CustomerOrdersWidget* ui;
    QSqlQueryModel* model;
};

class CustomerOrdersDockWidget : public QDockWidget {
public:
    explicit CustomerOrdersDockWidget(Database*, QWidget* =nullptr);
    ~CustomerOrdersDockWidget();
};

#endif