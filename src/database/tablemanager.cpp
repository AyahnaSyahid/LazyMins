#include "tablemanager.h"
#include "database.h"
#include <QSqlTableModel>
#include <QSqlRecord>

#include <QtDebug>


TableManager::TableManager(const QString& tn, Database* parent) : 
db(parent), QObject(parent) {
    tableModel = db->getTableModel(tn);
    if(!tableModel) {
        qDebug() << "Failed to find '" << tn << "' table";
    } else {
        tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    }
}

TableManager::~TableManager() {}

QSqlRecord TableManager::record() const {
    return tableModel->record();
}
