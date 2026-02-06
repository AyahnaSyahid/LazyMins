#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSettings>
#include <QHash>
#include "src/database/databasemanager.h"

#include <QtDebug>

int main(int argc, char **args)
{
  QApplication app(argc, args);
  app.setOrganizationName("AksaraJaya");
  app.setApplicationName("LazyAdmins");
  
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings settings;
  auto db = DatabaseManager::instance();

  return 0;
};