#include "database.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QFileInfo>
#include <QFile>
#include <QDir>

Database::Database(QObject* parent) : QObject(parent)
{
    QDir appDir = qApp->applicationDirPath();
    QString baseTarget = appDir.absoluteFilePath("data/database.sqlite3");
    if(!QFileInfo::exists(baseTarget) ) {
        appDir.mkpath("data");
        auto copy_ok = QFile::copy(":/db/base.db3", baseTarget);
        QFile f(baseTarget);
        f.setPermissions(QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
    }
    QSqlDatabase base = QSqlDatabase::addDatabase("QSQLITE");
    base.setDatabaseName(baseTarget);
    base.open();
    base.exec("PRAGMA foreign_keys = ON");
}

Database::~Database() {
    QSqlDatabase::database().close();
}