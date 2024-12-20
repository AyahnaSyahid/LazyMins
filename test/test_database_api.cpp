#include "invoiceitem.h"
#include "orderitem.h"
#include "useritem.h"
#include "customeritem.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtDebug>


int main(int argc, char** args) {
    auto base = QSqlDatabase::addDatabase("QSQLITE");
    base.setDatabaseName("sample.db");
    base.open()
    
}