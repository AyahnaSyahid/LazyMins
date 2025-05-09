#include "ordermanager.h"
#include "createorderdialog.h"
#include "editorderdialog.h"
#include "createorderdialog.h"


OrderManager::OrderManager(QObject* parent) : TableManager("orders", parent) {}
OrderManager::~OrderManager() {}

QDialog* OrderManager::createDialog(QWidget* parent) {
    CreateOrderDialog* cod = new CreateOrderDialog(parent);
    connect(cod, &CreateOrderDialog::accepted, this, &OrderManager::created);
    return cod;
}

QDialog* OrderManager::editDialog(int oid, QWidget* parent) {
    EditOrderDialog* eod = new EditOrderDialog(oid, parent);
    connect(eod, &EditOrderDialog::accepted, this, &OrderManager::modified);
    return eod;
}

void OrderManager::createOrder() {
    auto cod = createDialog();
    cod->open();
}

void OrderManager::editOrder(int oid) {
    
}