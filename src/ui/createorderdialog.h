#ifndef CreateOrderDialog_H
#define CreateOrderDialog_H

#include <QDialog>


class QDate;
class QSqlRecord;
class QSqlError;
class Database;
namespace Ui {
    class CreateOrderDialog;
}

class CreateOrderDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateOrderDialog(Database* db, QWidget* =nullptr);
    ~CreateOrderDialog();

public slots:
    void setCurrentOrderDate(const QDate&);
    // void onInsertStatus(const QSqlError&, const QSqlRecord&);
    // void editOrder();

private slots:
    void on_pickDate_clicked();
    void on_resetButton_clicked();
    void on_draftButton_clicked();
    void on_createPaymentButton_clicked();
    void on_customerBox_currentIndexChanged(int);
    void on_customerBox_customContextMenuRequested(const QPoint&);
    void on_productBox_customContextMenuRequested(const QPoint&);
    void on_unpaidTableView_doubleClicked(const QModelIndex&);
    void on_unpaidTableView_customContextMenuRequested(const QPoint&);
    void on_actionFindCustomer_triggered();
    void on_actionFindProduct_triggered();
    void updateSubTotal();
    void createProduct();
    void createCustomer();
    void changeProduct(int);
    void updateLSum();
    void onInsertSuccess();
    void resetProductIndex();
    void resetCustomerIndex();

signals:
    void insertSuccess();

protected:
    Ui::CreateOrderDialog* ui;
    Database* db;
};

#endif // CreateOrderDialog_H