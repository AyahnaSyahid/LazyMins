
#include "src/database/databasemanager.h"
#include "src/managers/basemanager.h"
#include "src/dialogs/logindialog.h"
#include "src/dialogs/createuserdialog.h"
#include "src/dialogs/edituserdialog.h"

#include <QtDebug>
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
  
  CreateUserDialog ld;
  ld.exec();
  
  EditUserDialog eud("nurholis");
  eud.exec();
  
  
  return 0;
};