#include "ordermanager.h"
#include "createorderdialog.h"
#include "editorderdialog.h"
#include "createorderdialog.h"

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

OrderManager::OrderManager(QObject* parent) : TableManager("orders", parent) {}
OrderManager::~OrderManager() {}

QDialog* OrderManager::createDialog(QWidget* parent) {
    CreateOrderDialog* cod = new CreateOrderDialog(parent);
    connect(cod, &CreateOrderDialog::accepted, this, &OrderManager::created);
    connect(cod, &CreateOrderDialog::queryInsert, this, &OrderManager::insertRecord);
    cod->connect(this, &OrderManager::insertStatus, cod, &CreateOrderDialog::queryStatus);
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

void OrderManager::insertRecord(const QSqlRecord& rc) {
    QSqlTableModel model;
    model.setTable(tableName());
    model.insertRecord(-1, rc);
    emit insertStatus(model.lastError(), rc);
    if(!model.lastError().isValid()) {
        emit created();
    }
}