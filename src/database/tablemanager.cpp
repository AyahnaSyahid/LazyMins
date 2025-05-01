#include "tablemanager.h"
#include <QSqlQuery>
#include <QSqlError>

TableManager::TableManager(const QString& tn, QObject* parent) : tableName(tn), QObject(parent)
{
    
}

TableManager::~TableManager() {}

bool TableManager::insert(const QVariantMap& param) {
    if(tableName.isEmpty()) return false;
}

bool TableManager::update(const QVariantMap& which, const QVariantMap& value) {
    if(tableName.isEmpty()) return false;
    
}

bool TableManager::erase(const QVariantMap& param) {
    if(tableName.isEmpty()) return false;
}