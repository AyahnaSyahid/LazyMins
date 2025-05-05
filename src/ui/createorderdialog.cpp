#include "createorderdialog.h"
#include "files/ui_createorderdialog.h"

CreateOrderDialog::CreateOrderDialog(QWidget* parent) :
ui(new Ui::CreateOrderDialog), QDialog(parent) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

CreateOrderDialog::~CreateOrderDialog() {
    delete ui;
}
