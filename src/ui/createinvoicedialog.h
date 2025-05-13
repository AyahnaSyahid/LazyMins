#ifndef CreateInvoiceDialog_H
#define CreateInvoiceDialog_H

#include <QDialog>

namespace Ui {
    class CreateInvoiceDialog;
}

class QItemSelectionModel;
class CreateOrderModel;
class CreateInvoiceDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateInvoiceDialog(QItemSelectionModel*, QWidget* = nullptr);
    ~CreateInvoiceDialog();

private:
    QItemSelectionModel* sm;
    CreateOrderModel* om;
    Ui::CreateInvoiceDialog* ui;
};

#endif
