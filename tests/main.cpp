#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSettings>
#include "src/database/databasemanager.h"
#include "src/models/basemanager.h"
#include "src/models/adminmanager.h"
#include "src/models/konsumenmanager.h"
#include "src/utils/authmanager.h"

#include <QtDebug>

int main(int argc, char **args)
{
  QCoreApplication app(argc, args);
  app.setOrganizationName("AksaraJaya");
  app.setApplicationName("LazyAdmins");
  
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings userSettings;
  
  auto sb = QSqlDatabase::addDatabase("QSQLITE", "LMAdmins_db");
  sb.setDatabaseName(":memory:");
  sb.open();
  
  auto &db = DatabaseManager::instance();
  if (sb.tables().count() < 5)
  {
    qDebug() << QString("%1 tables created").arg(sb.tables().count());
  }
  BaseManager::connection = QSqlDatabase::database("LMAdmins_db");
  AdminManager am;
  KonsumenManager km;
  QList<QSqlRecord> recordList;
  for(int i=1; i<201; ++i) {
    auto rc = km.create({{"nama_lengkap", QString("KONS-%1").arg(i, 4, 10, QChar('0'))}});
    if(rc.value("id").toInt() > 0) {
      recordList << rc;
    }
  }
  qDebug() << recordList.count() << "konsumen created";
  qDebug() << km.getAll().count() << "konsumen available on database";
  
  for(int i=0; i<200; i += 2) {
    km.remove(i);
  }

  for( const auto r : km.getAll()) {
    qDebug() << r.value("customer_code").toString() << r.value("nama_lengkap").toString();
  }
  
  return 0;
};