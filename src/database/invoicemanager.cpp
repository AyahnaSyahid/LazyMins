#include "invoicemanager.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

InvoiceManager::InvoiceManager(QObject *parent) :
TableManager("invoices", parent) {
    QSqlTableModel* model = new QSqlTableModel(this);
    model->setObjectName("model");
    model->setTable(tableName());
    model->setEditStrategy(model.OnManualSubmit);
    model->select();
}

InvoiceManager::~InvoiceManager(){}

void InvoiceManager::insertRecord(const QSqlRecord& rc) {
    auto model = findChild<QSqlTableModel*>("model");
    bool iok = model->insertRecord(-1, rc);
    emit insertStatus(model->las)
}

void InvoiceManager::updateRecord(const QSqlRecord& rc) {
    auto model = findChild<QSqlTableModel*>("model");
    
}

void InvoiceManager::removeRecord(const QSqlRecord& rc) {
    auto model = findChild<QSqlTableModel*>("model");
    
}
