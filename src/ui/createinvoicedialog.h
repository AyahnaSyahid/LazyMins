#ifndef CreateInvoiceDialog_H
#define CreateInvoiceDialog_H

#include <QDialog>

namespace Ui {
    class CreateInvoiceDialog;
}

class QItemSelectionModel;
class CreateOrderModel;
class Database;
class CreateInvoiceDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateInvoiceDialog(QItemSelectionModel*, Database* db, QWidget* = nullptr);
    ~CreateInvoiceDialog();

public slots:
    void calculateTotal();

private slots:
    void on_cancelButton_clicked();
    void on_saveButton_clicked();
    void on_payButton_clicked();

private:
    QList<int> orderIds;

private:
    QItemSelectionModel* sm;
    CreateOrderModel* om;
    Ui::CreateInvoiceDialog* ui;
    Database* db;
};

#endif
