#include "createproductdialog.h"
#include "files/ui_createproductdialog.h"

CreateProductDialog::CreateProductDialog(QWidget* parent) :
ui(new Ui::CreateProductDialog), QDialog(parent) {
    ui->setupUi(this);
<<<<<<< HEAD
    setAttribute(Qt::WA_DeleteOnClose);
=======
>>>>>>> refs/remotes/origin/new-start
}

CreateProductDialog::~CreateProductDialog() {
    delete ui;
}

void CreateProductDialog::on_pushButton_clicked() {
    
}