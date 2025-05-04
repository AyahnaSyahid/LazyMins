#include <QApplication>
#include <QMainWindow>
#include "adminwindow.h"
#include "usermanager.h"
#include "database.h"
#include "loginform.h"
#include "customermanager.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSqlTableModel>
#include <QTableView>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Database database;
    UserManager uman;

    if(!uman.nameExists("root")) {
        uman.createUser("root", "holis", "Na Ha La Ka Ma Ra Da");
    }
    
    QMainWindow mainWindow;
    auto menuBar = mainWindow.menuBar();
    
    CustomerManager cm;
    QTableView tv;
    QSqlTableModel md;
    md.setTable("customers");
    tv.setModel(&md);
    md.select();
    
    cm.connect(&cm, &CustomerManager::newCustomer, &md, &QSqlTableModel::select);
    cm.connect(&cm, &CustomerManager::editedCustomer, &md, &QSqlTableModel::select);
    
    auto customerMenu = menuBar->addMenu("CustomerMenu");
    auto actaddcustomer = customerMenu->addAction(mainWindow.tr("Daftarkan Konsumen"));
    
    cm.connect(actaddcustomer, &QAction::triggered, &cm, &CustomerManager::createCustomer);
    
    LoginForm lf(&uman);
    lf.connect(&lf, &QDialog::accepted, &mainWindow, &QMainWindow::show);
    lf.connect(&lf, &QDialog::accepted, &tv, &QTableView::show);
    lf.open();
    
    
    return app.exec();
}