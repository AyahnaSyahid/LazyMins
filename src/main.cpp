
#include "src/database/databasemanager.h"
#include "src/managers/basemanager.h"
#include "src/mainwindow/mainwindow.h"

#include <QtDebug>
#include <QSqlRecord>
#include <QApplication>
#include <QSettings>
#include <QVariantMap>
#include <QMessageBox>
#include <QTimer>

namespace GLB {
  void setup(QVariantMap &);
}

int main(int argc, char **args)
{
  QApplication app(argc, args);
  Q_INIT_RESOURCE(database_resources);
  
  app.setOrganizationName("AksaraJaya");
  app.setApplicationName("LazyAdmins");
  
  QSettings::setDefaultFormat(QSettings::IniFormat);
  
  QVariantMap setup_result;
  GLB::setup(setup_result);
  
  if(!setup_result["success"].toBool()) {
    QMessageBox::critical(nullptr, "Setup dibatalkan", setup_result["setup_message"].toString());
    app.quit();
    return 1;
  }
  
  QSqlDatabase sb = QSqlDatabase::addDatabase("QSQLITE");
  if (setup_result.value("run_init").toBool()) {
    QSettings s;
    sb.setDatabaseName(s.value("Database/database_file").toString());
    if (!sb.open()) {
      qDebug() << "Database not open" << sb.databaseName();
      app.quit();
      return 0;
    }
    DatabaseManager::initSchema(sb);
    // add super_admin
    
  }
  
  auto &db = DatabaseManager::instance();
  db.setDatabase(sb);

  BaseManager::connection = db.database();

  MainWindow mw;
  QTimer::singleShot(0, &mw, &MainWindow::openLoginForm);

  app.exec();
  return 0;
};