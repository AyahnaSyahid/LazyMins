#include "invoicesmanagerdialog.h"
#include "files/ui_invoicesmanagerdialog.h"
#include "database.h"
#include "models/invoicesmanagermodel.h"


InvoicesManagerDialog::InvoicesManagerDialog(Database *_d, QWidget* parent) :
db(_d), inModel(new InvoicesManagerModel(_d, this)), ui(new Ui::InvoicesManagerDialog), QDialog(parent) {
    ui->setupUi(this);
    ui->invoicesView->setModel(inModel);
}

InvoicesManagerDialog::~InvoicesManagerDialog() {
    delete ui;
}
