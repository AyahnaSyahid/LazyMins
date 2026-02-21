#include <QApplication>
#include <QSettings>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include "src/widget/jinvmainwindow.h"
#include "src/widget/konfigurasi.h"

bool initializeDatabase();

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  app.setOrganizationName("AksaraJaya");
  app.setApplicationName("Just-Invoice");

  Q_INIT_RESOURCE(dbres);
  
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings s(QSettings::UserScope);
  if (!QFileInfo::exists(s.fileName())) {
    qDebug() << "First time";
    Konfigurasi k;
    if (k.exec() == QDialog::Accepted) {
      if(initializeDatabase()) {
        QMessageBox::information(nullptr, "Restart Aplikasi", "Inisialisasi database berhasil\nMulai ulang aplikasi secara manual");
        app.quit();
        return 0;
      } else {
        QMessageBox::information(nullptr, "Gagal", "Inisialisasi database gagal ulang aplikasi secara manual");
        app.quit();
        return 0;  
      }
    } else {
      QMessageBox::critical(nullptr, "Error", "Tidak dapat melanjutkan tanpa database");
      return 1;
    }
  }
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(s.value("Database/Path").toString());
  if( !db.open() ) {
    QMessageBox::critical(nullptr, "Error", "Tidak dapat menyambungkan ke Database");
    app.quit();
    return 1;
  }
  
  QSqlQuery q("PRAGMA foreign_keys = ON");
  
  JINVMainWindow main;
  main.show();
  
  return app.exec();
}