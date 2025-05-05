#include "createorderdialog.h"
#include "files/ui_createorderdialog.h"

CreateOrderDialog::CreateOrderDialog(QWidget* parent) :
ui(new Ui::CreateOrderDialog), QDialog(parent) {
    ui->setupUi(this);
}

CreateOrderDialog::~CreateOrderDialog() {
    delete ui;
}
