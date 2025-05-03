#include "ordermanager.h"
#include <QDialog>

OrderManager::OrderManager(QObject* parent) : TableManager("orders", parent) {}
OrderManager::~OrderManager() {}

QDialog* OrderManager::createDialog(QWidget* parent) {
    return QDialog(parent);
}

QDialog* OrderManager::editDialog(QWidget* parent) {
    return QDialog(parent);
}