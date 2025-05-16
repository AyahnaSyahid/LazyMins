#include "database.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QtDebug>

Database::Database(QObject* parent) : tModels(), QObject(parent)
{
    QDir appDir = qApp->applicationDirPath();
    QString baseTarget = appDir.absoluteFilePath("data/" APP_DATABASE_NAME);
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
    auto tables = base.tables(QSql::Tables);
    QSqlTableModel* model=nullptr;
    for(auto name = tables.cbegin(); name != tables.cend(); ++name) {
        model = new QSqlTableModel(this);
        model->setEditStrategy(QSqlTableModel::OnManualSubmit);
        model->setTable(*name);
        model->setObjectName(QString("%1_tableModel").arg(*name));
        model->select();
        tModels.insert(*name, model);
    }
}

Database::~Database() {
    QSqlDatabase::database().close();
}

QSqlTableModel* Database::getTableModel(const QString& name) {
    if(tModels.contains(name)) {
        return tModels.value(name);
    }
    return nullptr;
}