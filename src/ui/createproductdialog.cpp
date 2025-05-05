#include "createproductdialog.h"
#include "files/ui_createproductdialog.h"

CreateProductDialog::CreateProductDialog(QWidget* parent) :
ui(new Ui::CreateProductDialog), QDialog(parent) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

CreateProductDialog::~CreateProductDialog() {
    delete ui;
}

void CreateProductDialog::on_pushButton_clicked() {
    
}