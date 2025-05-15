#include "customermanager.h"
#include "createcustomerdialog.h"
#include "editcustomerdialog.h"
#include "database.h"
#include <QDialog>

CustomerManager::CustomerManager(Database* parent) :
TableManager("customers", parent) {
    
}

CustomerManager::~CustomerManager(){}

QDialog* CustomerManager::createDialog(QWidget* parent) {
    CreateCustomerDialog *ccd = new CreateCustomerDialog(parent);
    connect(ccd, &QDialog::accepted, this, &CustomerManager::created);
    return ccd;
}

QDialog* CustomerManager::editDialog(int cid, QWidget* parent) {
    EditCustomerDialog *ecd = new EditCustomerDialog(cid, parent);
    connect(ecd, &QDialog::accepted, this, &CustomerManager::modified);
    return ecd;
}

void CustomerManager::createCustomer() {
    auto cd = createDialog();
    cd->open();
}

void CustomerManager::editCustomer(int cid) {
    auto ed = editDialog(cid);
    ed->open();
}