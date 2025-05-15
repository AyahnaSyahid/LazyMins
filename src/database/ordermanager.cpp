#include "ordermanager.h"
#include "createorderdialog.h"
#include "editorderdialog.h"
#include "createorderdialog.h"
#include "database.h"

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

OrderManager::OrderManager(Database* _db) :
TableManager("orders", _db) {
    auto customerModel = db->getTableModel("customers");
    auto productModel = db->getTableModel("products");
    connect(customerModel, &QAbstractItemModel::dataChanged, tableModel, &QSqlTableModel::select);
    connect(productModel, &QAbstractItemModel::dataChanged, tableModel, &QSqlTableModel::select);
}

OrderManager::~OrderManager() {}

QDialog* OrderManager::createDialog(QWidget* parent) {
    CreateOrderDialog* cod = new CreateOrderDialog(db, parent);
    cod->connect(cod, &CreateOrderDialog::queryInsert, this, &OrderManager::insertRecord);
    // cod->connect(cod, &CreateOrderDialog::queryUpdate, this, &OrderManager::updateRecord);
    cod->connect(this, &OrderManager::insertStatus, cod, &CreateOrderDialog::onInsertStatus);
    return cod;
}

QDialog* OrderManager::editDialog(int oid, QWidget* parent) {
    tableModel->setFilter(QString("order_id=%1").arg(oid));
    auto rec = tableModel->record(0);
    tableModel->setFilter("");
    return editDialog(rec, parent);
}

QDialog* OrderManager::editDialog(const QSqlRecord& rc, QWidget* parent) {
    EditOrderDialog* eo = new EditOrderDialog(rc, db, parent);
    // cod->connect(eo, &EditOrderDialog::queryInsert, this, &OrderManager::insertRecord);
    eo->connect(eo, &EditOrderDialog::queryUpdate, this, &OrderManager::updateRecord);
    eo->connect(this, &OrderManager::updateStatus, eo, &EditOrderDialog::onUpdateStatus);
    return eo;
}

void OrderManager::createOrder() {
    auto d = createDialog();
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->open();
}

void OrderManager::editOrder(int oid) {
    auto e = editDialog(oid);
    e->setAttribute(Qt::WA_DeleteOnClose);
    e->open();
}

void OrderManager::insertRecord(const QSqlRecord& rc) {
    if(tableModel->insertRecord(-1, rc)) {
        if(!tableModel->submitAll()) {
            tableModel->revertAll();
        }
        emit insertStatus(tableModel->lastError(), rc);
        return;
    }
    emit insertStatus(QSqlError("ApplicationError", "Unable to insert record", QSqlError::UnknownError), rc);    
}

void OrderManager::updateRecord(const QSqlRecord& rc) {
    qint64 oid = rc.value("order_id").toLongLong();
    if(oid > 0) {
        tableModel->setFilter(QString("order_id = %1").arg(oid));
        if(tableModel->setRecord(0, rc)) {
            if(!tableModel->submitAll()) {
                tableModel->revertAll();
            }
            emit updateStatus(tableModel->lastError(), rc);
            return;
        }
    }
    emit updateStatus(QSqlError("ApplicationError", "Invalid order_id value", QSqlError::UnknownError), rc);
}