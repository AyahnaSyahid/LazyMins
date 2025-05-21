#include "invoicesmanagerdialog.h"
#include "files/ui_invoicesmanagerdialog.h"
#include "database.h"

InvoicesManagerDialog::InvoicesManagerDialog(Database *_d, QWidget* parent) :
db(_d), ui(new Ui::InvoicesManagerDialog), QDialog(parent) {
    ui->setupUi(this);
}

InvoicesManagerDialog::~InvoicesManagerDialog() {
    delete ui;
}
