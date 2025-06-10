#ifndef CustomerOrdersWidget_H
#define CustomerOrdersWidget_H

#include <QWidget>
#include <QDockWidget>

namespace Ui {
    class CustomerOrdersWidget;
};

class Database;
class QSqlQueryModel;
class MainWindow;
class CustomerOrdersWidget : public QWidget {
    Q_OBJECT
public:
    explicit CustomerOrdersWidget(Database*, QWidget* =nullptr);
    ~CustomerOrdersWidget();

private slots:
    void reloadData();
	void on_customerOrdersTable_customContextMenuRequested(const QPoint&);
	void showOrdersFor(const QVariant&);
	void showInvoicesFor(const QVariant&);

signals:
    void createInvoiceForOrders(const QList<int>&);
    void createInvoiceForOrdersSent();
    void editOrder(int);

private:
    Ui::CustomerOrdersWidget* ui;
    QSqlQueryModel* model;
};

class CustomerOrdersDockWidget : public QDockWidget {
public:
    explicit CustomerOrdersDockWidget(Database*, MainWindow* =nullptr);
    ~CustomerOrdersDockWidget();
};

#endif