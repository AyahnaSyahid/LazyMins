#include <QApplication>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QLocale>
#include <QMessageBox>

#include "config.h"
#include "mainwindow.h"
#include "logindialog.h"
#include "adduserdialog.h"
#include "devmode.h"

namespace GlobalNamespace {
    qint64 logged_user_id = -2000;
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  
  // Setup Here
  app.setApplicationName(QAPPNAME);
  app.setOrganizationName(QAPPORG);
  QLocale::setDefault(QLocale("id_ID"));
  
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.applicationDirPath());
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings st;
  st.setValue("Test/Welcome", "Test Nya");
  st.sync();
  
  // Database
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("sample.db");
  bool dbOk = db.open();
  if(dbOk){
      QSqlQuery e;
      bool ok = e.exec("PRAGMA foreign_keys = ON");
      if(!ok) {
        qDebug() << "Failed to set ForeignKey";
      } else {
        qDebug() << "ForeignKey Sets";
      }
  }
  
  if(!dbOk) {
      QMessageBox msgb;
      msgb.setStandardButtons(QMessageBox::Yes);
      msgb.setButtonText(QMessageBox::Yes, "Ya");
      msgb.exec();
      return 1;
  }
  
  DB_INIT::initializeDatabase();
  DB_INIT::fillDummyDatabaseData();
  
  auto id = LoginDialog::blockingLoginDialog();
  if(id < 1) {
      app.exit(1);
      return 1;
  }
  qDebug() << "Test FLOOR, CEIL";
  QSqlQuery q("SELECT FLOOR(3.99) AS [floor(3.99)], CEIL(5.0001) AS [ceil(5.0001)]");
  if(q.next()) {
    auto rec = q.record();
    for(int c=0; c<rec.count(); ++c)
        qDebug() << rec.fieldName(c) << " = " << q.value(c);
  } else {
    qDebug() << "Error occured :" << q.lastError().text();
  }
  MainWindow mainWindow;
  mainWindow.show();

  return app.exec();
}