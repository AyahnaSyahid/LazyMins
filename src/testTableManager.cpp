#include "database/tablemanager.h"
#include <QtDebug>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

void debugError(const QSqlError& ee) {
    qDebug() << ee;
} 

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QString dbPath = QCoreApplication::applicationDirPath() + "/" + "testBase.db3";
    qDebug() << dbPath;
    QFile::copy(":/db/base.db3", dbPath);
    QFile f(dbPath);
    bool ok = f.setPermissions(QFile::ReadOther | QFile::ExeOther | QFile::WriteOther);
    qDebug() << "Permission Sets ? " << ok;
    auto base = QSqlDatabase::addDatabase("QSQLITE");
    base.setDatabaseName(dbPath);
    base.open();
    
    TableManager cManager("customers"),
                 pManager("products"),
                 uManager("users"),
                 oManager("orders");
    
    cManager.setObjectName("cManager");
    pManager.setObjectName("pManager");
    uManager.setObjectName("uManager");
    oManager.setObjectName("oManager");
    
    cManager.connect(&cManager, &TableManager::error, &debugError);
    pManager.connect(&pManager, &TableManager::error, &debugError);
    uManager.connect(&uManager, &TableManager::error, &debugError);
    oManager.connect(&oManager, &TableManager::error, &debugError);
    
    bool acs = false,
         aps = false,
         aus = false,
         aos = false;
    
    aus = uManager.insert({{"account_name","Admin01"}, {"password_hash", "Admin01PassHash"},{"salt", "dummy_salt"}, {"display_name", "01-ADMIN"}}
        );
    
    qDebug() << "Add User : " << (aus ? "Ok" : "Fail");
    
    if(!aus) {
        qDebug() << uManager.lastError();
    }
    
    acs = cManager.insert({{"name", "Abah Senara"},{"address","Jl Mana"}, {"phone", "0853322678"}});
    qDebug() << "Add Customer : " << (acs ? "Ok" : "Fail");
    if(!acs) {
        qDebug() << cManager.lastError();
        return 0;
    }
    
    aps = pManager.insert({{"name", "C-A3P-AP150"},
                           {"description","Cetak A3Plus Bahan AP150"},
                           {"base_price", 2500},
                           {"material", "Kertas"},
                           {"category", "Print"},
                           });
    
    qDebug() << "Add Product : " << (aps ? "Ok" : "Fail");
    if(!aps) {
        qDebug() << pManager.lastError();
        return 0;
    }
    
    aos = oManager.insert({
        {"user_id", 1},
        {"customer_id", 1},
        {"product_id", 1}, 
        {"use_area", 0}, 
        {"width", 1}, 
        {"height", 1}, 
        {"quantity", 1}, 
        {"unit_price", 2500} });
        
    qDebug() << "Add Order : " << (aos ? "Ok" : "Fail");
    if(!aos) {
        qDebug() << oManager.lastError();
        return 0;
    }
    
    
    return 0;
}