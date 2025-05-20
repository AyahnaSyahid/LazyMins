#ifndef CreatePaymentDialog_H
#define CreatePaymentDialog_H

namespace Ui {
    class CreatePaymentDialog;
}

#include <QDialog>

class Database;
class QSortFilterProxyModel;
class CreatePaymentDialog : public QDialog {
    Q_OBJECT

public :
    explicit CreatePaymentDialog(int, Database*, QWidget* =nullptr);
    ~CreatePaymentDialog();

private slots:
    void inputLogic();
    void fillUiData();
    void on_openOrdersView_clicked();
    void on_saveButton_clicked();
    
private:
    int invoiceId;
    Database* db;
    QSortFilterProxyModel* paymentHistoryModel;
    Ui::CreatePaymentDialog* ui;
};

#endif