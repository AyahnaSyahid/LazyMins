#include "ordermanager.h"
#include "createorderdialog.h"
#include "editorderdialog.h"
#include "createorderdialog.h"

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

OrderManager::OrderManager(QObject* parent) : TableManager("orders", parent) {}
OrderManager::~OrderManager() {}

QDialog* OrderManager::createDialog(QWidget* parent) {
    CreateOrderDialog* cod = new CreateOrderDialog(parent);
    connect(cod, &CreateOrderDialog::accepted, this, &OrderManager::orderCreated);
    connect(cod, &CreateOrderDialog::queryInsert, this, &OrderManager::insertRecord);
    connect(cod, &CreateOrderDialog::queryUpdate, this, &OrderManager::updateRecord);
    // tujuan connect menggunakan cod supaya tidak error saat cod dihapus
    cod->connect(this, &OrderManager::insertStatus, cod, &CreateOrderDialog::insertStatus);
    cod->connect(this, &OrderManager::updateStatus, cod, &CreateOrderDialog::updateStatus);
    cod->connect(cod, &CreateOrderDialog::customerCreated, this, &OrderManager::customerCreated);
    cod->connect(cod, &CreateOrderDialog::productCreated, this, &OrderManager::productCreated);
    cod->connect(cod, &CreateOrderDialog::orderModified, this, &OrderManager::orderModified);
    return cod;
}

QDialog* OrderManager::editDialog(int oid, QWidget* parent) {
    QSqlQuery q;
    q.prepare("SELECT * FROM orders WHERE order_id = ?");
    q.addBindValue(oid);
    q.exec() && q.next();
    EditOrderDialog* eod = new EditOrderDialog(q.record(), parent);
    connect(eod, &EditOrderDialog::accepted, this, &OrderManager::orderModified);
    return eod;
}

QDialog* OrderManager::editDialog(const QSqlRecord& rc, QWidget* parent) {
    EditOrderDialog* eod = new EditOrderDialog(rc, parent);
    connect(eod, &EditOrderDialog::accepted, this, &OrderManager::orderModified);
    return eod;
}

void OrderManager::createOrder() {
    auto cod = createDialog();
    cod->open();
}

void OrderManager::editOrder(int oid) {
    auto eod = editDialog(oid, nullptr);
    eod->open();
}

void OrderManager::insertRecord(const QSqlRecord& rc) {
    QSqlTableModel model;
    model.setTable(tableName());
    model.insertRecord(-1, rc);
    emit insertStatus(model.lastError(), rc);
    if(!model.lastError().isValid()) {
        emit orderCreated();
    }
}

void OrderManager::updateRecord(const QSqlRecord& rc) {
    QSqlTableModel model;
    model.setTable(tableName());
    model.setFilter(QString("order_id = %1").arg(rc.value("order_id").toLongLong()));
    model.select();
    model.setEditStrategy(model.OnManualSubmit);
    model.setRecord(0, rc);
    model.submitAll();
    if(!model.lastError().isValid()) {
        emit orderModified();
    }
    emit updateStatus(model.lastError(), rc);
}