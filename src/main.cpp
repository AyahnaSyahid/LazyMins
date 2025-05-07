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

void setupMain(QMainWindow *, QObject *);

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
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

void setupMain(QMainWindow *mw, QObject *parent) {
    
    ProductManager* pm = new ProductManager(parent);
    CustomerManager* cm = new CustomerManager(parent);
    OrderManager* om = new OrderManager(parent);
    
    mw->setMinimumWidth(960);
    mw->setMinimumHeight(600);
    
    auto menuBar = mw->menuBar();
    
    QSqlTableModel* pTable = new QSqlTableModel(parent);
    QSqlTableModel* oTable = new QSqlTableModel(parent);
    QSqlTableModel* cTable = new QSqlTableModel(parent);
    pTable->setTable("products");
    cTable->setTable("customers");
    oTable->setTable("orders");
    pTable->select();
    oTable->select();
    cTable->select();
    
    om->connect(om, &OrderManager::created, oTable, &QSqlTableModel::select);
    om->connect(om, &OrderManager::modified, oTable, &QSqlTableModel::select);
    pm->connect(pm, &ProductManager::created, pTable, &QSqlTableModel::select);
    pm->connect(pm, &ProductManager::modified, pTable, &QSqlTableModel::select);
    cm->connect(cm, &CustomerManager::created, cTable, &QSqlTableModel::select);
    cm->connect(cm, &CustomerManager::modified, cTable, &QSqlTableModel::select);
    
    QTableView *pTV = new QTableView(mw), *cTV = new QTableView(mw), *oTV = new QTableView(mw);
    pTV->setModel(pTable);
    oTV->setModel(oTable);
    cTV->setModel(cTable);
    
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