#include "ordersmodule.h"
#include "addorderdialog.h"
#include "customersorderdockwidget.h"
#include "managernotadockwidget.h"
#include "userpermissions.h"
#include "notifier.h"
#include "helper.h"
#include "pembuatnota.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QTableView>
#include <QMenu>
#include <QAction>

OrdersModule::OrdersModule(QObject* parent)
: QObject(parent) {
    mainWindow = qobject_cast<QMainWindow*>(parent);
}

void OrdersModule::installMenu(QMainWindow* mw_arg) {
    QMainWindow *mw = mainWindow;
    if(mw_arg)
        mw = mw_arg;
    auto menuBar = mw->menuBar();
    if(!menuBar) return;
    auto menuOrder = menuBar->addMenu("Order");
    menuOrder->setObjectName("menuOrder");
    menuOrder->setProperty("enableWithPerm", "Viewer");
    auto act = menuOrder->addAction("Baru");
    act->setObjectName("actionAddOrder");
    act->setProperty("enableWithPerm", "AddOrder");
    connect(act, &QAction::triggered, this, &OrdersModule::onAddOrder);
    act = menuOrder->addAction("Bayar");
    act->setObjectName("actionAddPayment");
    connect(act, &QAction::triggered, this, &OrdersModule::onCreatePayment);
}

void OrdersModule::installDocker(QMainWindow* mw_arg) {
    QMainWindow *mw = mainWindow;
    if(mw_arg)
        mw = mw_arg;
    auto cdock = new CustomersOrderDockWidget(mw);
    mw->addDockWidget(Qt::TopDockWidgetArea, cdock);
    auto menuView = mw->findChild<QMenu*>("menuView");
    auto toga = cdock->toggleViewAction();
    toga->setProperty("enableWithPerm", "Viewer");
    menuView->addAction(toga);
    auto tv = cdock->findChild<QTableView*>("tableView");
    if(tv) {
        auto bef = cdock->findChild<QAction*>("actionEditOrder");
        tv->insertAction(bef, mw->findChild<QAction*>("actionAddOrder"));
    }
    auto mnd = new ManagerNotaDockWidget(mw);
    mw->addDockWidget(Qt::TopDockWidgetArea, mnd);
    toga = mnd->toggleViewAction();
    toga->setProperty("enableWithPerm", "Viewer");
    menuView->addAction(toga);
    mnd->show();
}

void OrdersModule::onAddOrder() {
    if(!UserPermissions::hasPermission(PermissionItem("AddOrder"))) {
        MessageHelper::warning(nullptr, "Peringatan", "Anda tidak memiliki hak akses untum menambahkan order");
        return;
    }
    auto aod = new AddOrderDialog();
    aod->setAttribute(Qt::WA_DeleteOnClose);
    connect(aod, &AddOrderDialog::orderAdded, [](){ dbNot->emitChanged("orders"); });
    aod->open();
}

void OrdersModule::onCreatePayment() {
    auto dlg = new QDialog(mainWindow);
    auto l = new QVBoxLayout(dlg);
    auto pnot = new PembuatNota(dlg);
    l->addWidget(pnot);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open();
}

void OrdersModule::onModifyOrder() {}
