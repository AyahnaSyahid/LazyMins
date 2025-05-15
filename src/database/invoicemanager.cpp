#include "invoicemanager.h"
#include "database.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

InvoiceManager::InvoiceManager(Database *parent) :
TableManager("invoices", parent) {
    auto pr = parent->getTableModel("products");
    auto cs = parent->getTableModel("customers");
    auto od = parent->getTableModel("orders");
    
    connect(pr, &QSqlTableModel::dataChanged, tableModel, &QSqlTableModel::select);
    connect(cs, &QSqlTableModel::dataChanged, tableModel, &QSqlTableModel::select);
    connect(od, &QSqlTableModel::dataChanged, tableModel, &QSqlTableModel::select);
}

InvoiceManager::~InvoiceManager(){}

void InvoiceManager::insertRecord(const QSqlRecord& rc) {
}

void InvoiceManager::updateRecord(const QSqlRecord& rc) {
}

void InvoiceManager::removeRecord(const QSqlRecord& rc) {
}
