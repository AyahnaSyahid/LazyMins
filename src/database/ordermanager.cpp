#include "ordermanager.h"
#include "createorderdialog.h"
#include "editorderdialog.h"
#include "createorderdialog.h"
#include "database.h"

#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

OrderManager::OrderManager(Database* _db, QObject* parent) :
TableManager("orders", _db, parent) {
    auto customerModel = db->getTableModel("customers");
    auto productModel = db->getTableModel("products");
    connect(customerModel, &QAbstractItemModel::dataChanged, model(), &QSqlTableModel::select);
    connect(productModel, &QAbstractItemModel::dataChanged, model(), &QSqlTableModel::select);
}

OrderManager::~OrderManager() {}

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