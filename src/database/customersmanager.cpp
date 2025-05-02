#include "customersmanager.h"
#include <QSqlQuery>
#include <QSqlError>


CustomersManager::CustomersManager(QObject* parent) : QObject(parent) {}
CustomersManager::~CustomersManager(){}


bool CustomersManager::add(const QVariantMap& v) {
    QSqlQuery q;
    
}

bool CustomersManager::erase(const QVariantMap& v) {
    
}

bool CustomersManager::update(const QVariantMap& o, const QVariantMap& n) {
    
}