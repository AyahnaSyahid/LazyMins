#include <QApplication>
#include <QSqlDatabase>
#include <QSettings>
#include "src/database/databasemanager.h"
#include "src/models/adminmanager.h"

#include <QtDebug>

int main(int argc, char **args)
{
  QApplication app(argc, args);
  app.setOrganizationName("AksaraJaya");
  app.setApplicationName("LazyAdmins");
  
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings userSettings;
  
  auto sb = QSqlDatabase::addDatabase("QSQLITE", "LMAdmins_db");
  sb.setDatabaseName(QString("%1/data/lm.db").arg(app.applicationDirPath()));
  sb.open();
  
  auto &db = DatabaseManager::instance();
  AdminManager am;
  CreateAdminParams cap;
  cap.username = "noerc88";
  cap.nama_lengkap = "Noer Kholis Komarudin";
  cap.email = "Ayah.Syahid2017@gmail.com";
  cap.literal_password = "mejikuhibiniu";
  cap.nomor_telepon = "089932089675";
  
  auto rec = am.create(cap);
  
  qDebug() << rec;
  return 0;
};