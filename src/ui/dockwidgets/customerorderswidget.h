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
private:
    Ui::CustomerOrdersWidget* ui;
    QSqlTableModel* model;
};

class CustomerOrdersDockWidget : public QDockWidget {
    explicit CustomerOrdersDockWidget(Database*, QWidget* =nullptr);
    ~CustomerOrdersDockWidget();
};

#endif