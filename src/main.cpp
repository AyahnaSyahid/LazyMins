
#include "src/database/databasemanager.h"
#include "src/managers/basemanager.h"
#include "src/managers/konsumenmanager.h"
#include "src/dialogs/konsumendialog.h"


#include <QtDebug>
#include <QSqlRecord>
#include <QApplication>
#include <QSettings>

int main(int argc, char **args)
{
  QApplication app(argc, args);
  // app.setOrganizationName("AksaraJaya");
  // app.setApplicationName("LazyAdmins");
  
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings userSettings;
  
  auto sb = QSqlDatabase::addDatabase("QSQLITE");
  sb.setDatabaseName(QString("%1/../a.db").arg(app.applicationDirPath()));
  if (!sb.open()) {
    qDebug() << "Database not open" << sb.databaseName();
    app.quit();
    return 0;
  }
  
  auto &db = DatabaseManager::instance();
  db.setDatabase(sb);
  BaseManager::connection = db.database();
  KonsumenManager km;
  auto opt = km.getById(1);
  
  KonsumenDialog dialog;
  if(opt)
    dialog.prepareModify(*opt);
  else 
    dialog.prepareCreate();
  dialog.exec();
  return 0;
};