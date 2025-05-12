#ifndef CreateOrderDialog_H
#define CreateOrderDialog_H

#include <QDialog>


class QDate;
class QSqlRecord;
class QSqlError;
namespace Ui {
    class CreateOrderDialog;
}

class CreateOrderDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateOrderDialog(QWidget* = nullptr);
    ~CreateOrderDialog();

public slots:
    void setCurrentOrderDate(const QDate&);
    void queryStatus(const QSqlError&, const QSqlRecord&);
    void editOrder();

private slots:
    void on_pickDate_clicked();
    void on_resetButton_clicked();
    void on_draftButton_clicked();
    void on_createPaymentButton_clicked();
    
    void on_customerBox_customContextMenuRequested(const QPoint&);
    void on_productBox_customContextMenuRequested(const QPoint&);
    void on_unpaidTableView_doubleClicked(const QModelIndex&);
    void updateSubTotal();
    void updateOrdersModel();
    void createProduct();
    void createCustomer();
    void changeProduct(int);
    void onCustomerChanged(int);
    void updateLSum();

signals:
    void queryInsert(const QSqlRecord& rec);
    void customerCreated();
    void productCreated();

protected:
    Ui::CreateOrderDialog* ui;
};

#endif // CreateOrderDialog_H