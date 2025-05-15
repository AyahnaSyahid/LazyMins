#include <QApplication>
#include <QMainWindow>

#include "usermanager.h"
#include "database.h"
#include "loginform.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QLocale>

void setupMain(QMainWindow *, Database *);

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

    QMainWindow mainWindow;
    
    setupMain(&mainWindow, &database);
    
    LoginForm lf(&uman);
    lf.connect(&lf, &QDialog::accepted, &mainWindow, &QMainWindow::show);
    lf.open();
    
    return app.exec();
}

#include "ordermanager.h"
#include "customermanager.h"
#include "productmanager.h"

void setupMain(QMainWindow *mw, Database *base) {
    
    ProductManager* pm = new ProductManager(base);
    CustomerManager* cm = new CustomerManager(base);
    OrderManager* om = new OrderManager(base);
    
    mw->setMinimumWidth(960);
    mw->setMinimumHeight(600);
    
    auto menuBar = mw->menuBar();
    
    QTableView *pTV = new QTableView(mw), *cTV = new QTableView(mw), *oTV = new QTableView(mw);
    pTV->setModel(base->getTableModel("products"));
    oTV->setModel(base->getTableModel("orders"));
    cTV->setModel(base->getTableModel("customers"));
    
    QWidget *cw = new QWidget(mw);
    
    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(cTV);
    l->addWidget(pTV);
    l->addWidget(oTV);
    
    cw->setLayout(l);
    
    mw->setCentralWidget(cw);
    
    auto omenu = menuBar->addMenu(mw->tr("Order"));
    QAction *act = omenu->addAction("Catat Order");
    act->connect(act, &QAction::triggered, om, &OrderManager::createOrder);
    omenu = menuBar->addMenu(mw->tr("Konsumen"));
    act = omenu->addAction("Daftarkan Konsumen");
    act->connect(act, &QAction::triggered, cm, &CustomerManager::createCustomer);
    omenu = menuBar->addMenu(mw->tr("Produk"));
    act = omenu->addAction("Daftarkan Produk");
    act->connect(act, &QAction::triggered, pm, &ProductManager::createProduct);
}