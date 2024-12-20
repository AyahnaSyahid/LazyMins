#include <QAction>
#include <QDockWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QFont>
#include <QLabel>

#include "customermodule.h"
#include "notifier.h"
#include "userpermissions.h"
#include "addcustomerdialog.h"
#include "managercustomerdialog.h"
#include "tabelkonsumen.h"

CustomerModule::CustomerModule(QObject* mw) : QObject(mw) {
    mainWindow = qobject_cast<QMainWindow*>(mw);
}

void CustomerModule::installMenu(QMainWindow* mw) {
    QAction *addCustomer,
            *showManager,
            *showDocker;
    
    auto menuBar = mw->menuBar();
    auto customerMenu = menuBar->findChild<QMenu*>("menuCustomer");
    if(!customerMenu) {
        customerMenu = menuBar->addMenu("Konsumen");
        customerMenu->setObjectName("menuCustomer");
        // customerMenu->setProperty("enableWithPerm", "ManageCustomer");
    }
    addCustomer = customerMenu->addAction("Konsumen Baru");
    addCustomer->setObjectName("actionAddCustomer");
    showManager = customerMenu->addAction("Manager Konsumen");
    showManager->setObjectName("actionShowCustomerManager");
    
    addCustomer->setProperty("enableWithPerm", "ManageCustomer");
    showManager->setProperty("enableWithPerm", "ManageCustomer");
    
    connect(addCustomer, &QAction::triggered, this, &CustomerModule::onAddCustomer);
    connect(showManager, &QAction::triggered, this, &CustomerModule::onManageCustomer);
    
    auto viewMenu = menuBar->findChild<QMenu*>("menuView");
    if(!viewMenu){
        viewMenu = menuBar->addMenu("View");
        viewMenu->setObjectName("menuView");
    }
}

void CustomerModule::installDocker(QMainWindow* mw) {
    // auto dockCustomer = new QDockWidget(mw);
    // auto w = new TabelKonsumen(dockCustomer);
    // dockCustomer->setWidget(w);
    // dockCustomer->setWindowTitle("Pintasan Konsumen");
    // mw->addDockWidget(Qt::LeftDockWidgetArea, dockCustomer);
    // auto menuView = mw->findChild<QMenu*>("menuView");
    // if(menuView) {
        // menuView->addAction(dockCustomer->toggleViewAction());
        // dockCustomer->hide();
    // }
    // dockCustomer->setProperty("enableWithPerm", "ManageCustomer");
    // dockCustomer->toggleViewAction()->setProperty("enableWithPerm", "ManageCustomer");
}

void CustomerModule::onAddCustomer() {
    auto d = new AddCustomerDialog(mainWindow);
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->open();
}

void CustomerModule::onManageCustomer() {
    auto d = new ManagerCustomerDialog(mainWindow);
    connect(dbNot, &DatabaseNotifier::tableChanged, [d](QString s) { if(s == "customers") d->refreshModel();});
    connect(d, &ManagerCustomerDialog::customerRemoved, this, &CustomerModule::customerRemoved);
    auto act = mainWindow->findChild<QAction*>("actionAddCustomer");
    if(act) {
        auto tv = d->findChild<QTableView*>("tableView");
        if(tv) {
            tv->setContextMenuPolicy(Qt::ActionsContextMenu);
            tv->addAction(act);
            auto pushButton = d->findChild<QPushButton*>("pushButton");
            if(pushButton) {
                auto actDel = new QAction("Hapus", tv);
                connect(actDel, &QAction::triggered, pushButton, &QPushButton::clicked);
                tv->addAction(actDel);
            }
        }
    }
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->open();    
}