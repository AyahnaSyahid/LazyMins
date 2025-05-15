#include <QApplication>
#include <QMainWindow>

#include "usermanager.h"
#include "database.h"
#include "loginform.h"
#include "mainwindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QLocale>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QLocale loc(QLocale::Indonesian, QLocale::Indonesia);
    QLocale::setDefault(loc);
    Database database;
    UserManager uman;

    if(!uman.nameExists("root")) {
        uman.createUser("root", "holis", "Na Ha La Ka Ma Ra Da");
    }

    MainWindow mainWindow(&database);
    
    LoginForm lf(&uman);
    lf.connect(&lf, &QDialog::accepted, &mainWindow, &QMainWindow::show);
    lf.open();
    
    return app.exec();
}