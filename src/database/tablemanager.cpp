#include "tablemanager.h"
#include "database.h"
#include <QSqlTableModel>
#include <QSqlRecord>

#include <QtDebug>


TableManager::TableManager(const QString& tn, Database* _db, QObject* parent) : 
db(_db), _model(_db->getTableModel(tn)), QObject(parent) {}

TableManager::~TableManager() {}

QSqlRecord TableManager::record() const {
    return _model->record();
}

const QSqlTableModel* TableManager::model() {
    return _model;
}