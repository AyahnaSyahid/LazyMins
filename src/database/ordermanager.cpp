#include "ordermanager.h"
#include <QDialog>

OrderManager::OrderManager(QObject* parent) : TableManager("orders", parent) {}
OrderManager::~OrderManager() {}

QDialog* OrderManager::createDialog(QWidget* parent) {
    return new QDialog(parent);
}

QDialog* OrderManager::editDialog(QWidget* parent) {
    return new QDialog(parent);
}