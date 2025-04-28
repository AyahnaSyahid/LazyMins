#include "database/usermanager.h"
#include <QtDebug>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QString dbPath = QCoreApplication::applicationDirPath() + "/" + "testBase.db3";
    qDebug() << dbPath;
    QFile::copy(":/database/base.db3", dbPath);
    QFile f(dbPath);
    bool ok = f.setPermissions(QFile::ReadOther | QFile::ExeOther | QFile::WriteOther);
    qDebug() << "Permission Sets ? " << ok;
    auto base = QSqlDatabase::addDatabase("QSQLITE");
    base.setDatabaseName(dbPath);
    base.open();
    QString account_name = "holis",
            display_name = "Nur Holis",
            salt = UserManager::generateSalt();
    UserManager um;
    um.createUser("holis", "admin", "Na Ha La Ka Ma Ra Da");
    
    qDebug() << "Login Status ->" << um.login("holis", "adatist");
    qDebug() << "Login Status ->" << um.login("holis", "admin");
    app.quit();
    return 0;
}