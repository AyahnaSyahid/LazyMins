#include "createinvoicedialog.h"
#include "files/ui_createinvoicedialog.h"
#include "models/createordermodel.h"

#include <QItemSelectionModel>

CreateInvoiceDialog::CreateInvoiceDialog(QItemSelectionModel* sel, QWidget* parent) :
ui(new CreateInvoiceDialog), om(nullptr), sm(sel), QDialog(parent) {
    ui->setupUi(this);
    om = qobject_cast<CreateOrderModel*>(sel->model());
}

CreateInvoiceDialog::~CreateInvoiceDialog() {
    delete ui;
}

