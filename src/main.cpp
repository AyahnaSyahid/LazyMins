
#include "src/database/databasemanager.h"
#include "src/managers/basemanager.h"
#include "src/mainwindow/mainwindow.h"

#include <QtDebug>
#include <QSqlRecord>
#include <QApplication>
#include <QSettings>

int main(int argc, char **args)
{
  QApplication app(argc, args);
  Q_INIT_RESOURCE(database_resources);
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
  if (db.database().tables().count() < 5) {
    DatabaseManager::initSchema(sb);
  }
  BaseManager::connection = db.database();
  
  MainWindow mw;
  mw.show();
  
  app.exec();
  return 0;
};