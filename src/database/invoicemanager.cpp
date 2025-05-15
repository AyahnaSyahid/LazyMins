#include "invoicemanager.h"
#include "database.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

InvoiceManager::InvoiceManager(Database *_db, QObject* parent) :
TableManager("invoices", _db, parent) {
    auto pr = _db->getTableModel("products");
    auto cs = _db->getTableModel("customers");
    auto od = _db->getTableModel("orders");
    
    connect(pr, &QSqlTableModel::dataChanged, model(), &QSqlTableModel::select);
    connect(cs, &QSqlTableModel::dataChanged, model(), &QSqlTableModel::select);
    connect(od, &QSqlTableModel::dataChanged, model(), &QSqlTableModel::select);
}

InvoiceManager::~InvoiceManager(){}

void InvoiceManager::insertRecord(const QSqlRecord& rc) {
}

void InvoiceManager::updateRecord(const QSqlRecord& rc) {
}

void InvoiceManager::removeRecord(const QSqlRecord& rc) {
}
