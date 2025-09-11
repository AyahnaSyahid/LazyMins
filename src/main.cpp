#include <QAction>
#include <QApplication>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSqlTableModel>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>
#include <QtDebug>

#include "database.h"
#include "loginform.h"
#include "mainwindow.h"
#include "usermanager.h"
#include "invoicegraphics.h"

int main(int argc, char** argv) {
  if (argc > 1) {
    if (QString(argv[1]).toLower() == "adduser") {
      if (argc == 5) {
        Database base;
        UserManager uman(&base);
        if (uman.nameExists(argv[2])) {
          qInfo() << QString("Nama User '%1' telah digunakan").arg("test user");
        }
      }
    }
  }

  QApplication app(argc, argv);
  
  // InvoiceGraphics::
  
  QLocale loc(QLocale::Indonesian, QLocale::Indonesia);
  QLocale::setDefault(loc);

  Database database;

  UserManager uman(&database);

  uman.setObjectName("userManager");

  if (!uman.nameExists("root")) {
    uman.createUser("root", "holis", "Na Ha La Ka Ma Ra Da");
  }

  MainWindow mainWindow(&database);

  LoginForm lf(&uman);
  lf.connect(&lf, &QDialog::accepted, &mainWindow, &QMainWindow::show);

  lf.setWindowModality(Qt::ApplicationModal);
  lf.open();
  return app.exec();
}