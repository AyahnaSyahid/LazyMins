#include "customermanager.h"

#include <QDialog>

CustomerManager::CustomerManager(QObject* parent) : TableManager("costumers", parent) {}
CustomerManager::~CustomerManager(){}

QDialog* CustomerManager::createDialog(QWidget* parent) {
    
}

QDialog* CustomerManager::editDialog(QWidget* parent) {
    
}