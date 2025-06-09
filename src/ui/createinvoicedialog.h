#ifndef CreateInvoiceDialog_H
#define CreateInvoiceDialog_H

#include <QDialog>

namespace Ui {
    class CreateInvoiceDialog;
}

class QItemSelectionModel;
class CreateOrderModel;
class Database;
class QSqlError;
class CreateInvoiceDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateInvoiceDialog(int cid, QItemSelectionModel*, Database* db, QWidget* = nullptr);
    static CreateInvoiceDialog* fromOrderList(const QList<int>& oid, Database* db, QWidget* parent);

    ~CreateInvoiceDialog();

public slots:
    void calculateTotal();

private slots:
    void on_cancelButton_clicked();
    void on_saveButton_clicked();
    void on_payButton_clicked();

private:
    QList<int> orderIds;
    bool saveInvoice(QSqlError* = nullptr);

signals:
    void openPayment(int invoice_id);

private:
    QItemSelectionModel* sm;
    CreateOrderModel* om;
    Ui::CreateInvoiceDialog* ui;
    Database* db;
    int custId;
    int savedInvoiceId;
};

#endif
