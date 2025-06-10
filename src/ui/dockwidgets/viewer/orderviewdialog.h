#ifndef OrderViewDialog_H
#define OrderViewDialog_H

#include <QDialog>

class QTableView;
class QSqlQueryModel;

class OrderViewDialog : public QDialog {
    Q_OBJECT

public:
    OrderViewDialog(int cs_id, QWidget* parent=nullptr);
    ~OrderViewDialog();

signals:
    void createInvoiceForOrders(const QList<int>& );
    void editOrder(int);

public slots:
    void reloadData();

private slots:
    void show_tableView_contextMenu(const QPoint&);

private:
    QTableView* tableView;
    QSqlQueryModel* model;
    QString customerName;
};

#endif