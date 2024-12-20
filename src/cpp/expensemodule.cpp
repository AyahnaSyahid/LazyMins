#include <QAction>
#include <QMainWindow>
#include <QMenuBar>
#include <QTableView>
#include <QItemSelectionModel>
#include <QModelIndex>

#include "expensemodule.h"
#include "expenseitem.h"
#include "expensesdockwidget.h"
#include "expensesdockwidgetcontents.h"
#include "addexpensedialog.h"
#include "notifier.h"
#include "userpermissions.h"
#include "helper.h"

ExpenseModule::ExpenseModule(QObject *parent)
: QObject(parent) {
    mainWindow = qobject_cast<QMainWindow*>(parent);
}

void ExpenseModule::installMenu(QMainWindow* mw) {
    QAction *writeExpenseNote, *addNewDivision;
    auto menuBar = mw->menuBar();
    auto expenseMenu = menuBar->findChild<QMenu*>("menuExpense");
    if(!expenseMenu) {
        expenseMenu = menuBar->addMenu("Pengeluaran");
        expenseMenu->setProperty("enableWithPerm", "ManageExpenses");
        expenseMenu->setObjectName("menuExpense");
    }
    writeExpenseNote = expenseMenu->addAction("Buat");
    writeExpenseNote->setToolTip("Buat catatan pengeluaran");
    writeExpenseNote->setObjectName("actionWriteExpenseNote");
    writeExpenseNote->setProperty("enableWithPerm", "ManageExpenses");

    connect(writeExpenseNote, &QAction::triggered, this, &ExpenseModule::onWriteExpenseNote);
}

void ExpenseModule::installDocker(QMainWindow* mw) {
    auto expDock = new ExpensesDockWidget(mw);
    auto expConts = new ExpensesDockWidgetContents(expDock);
    expDock->setWidget(expConts);
    expDock->setProperty("enableWithPerm", "Viewer");
    mw->addDockWidget(Qt::RightDockWidgetArea, expDock);
    
    auto actCreate = mw->menuBar()->findChild<QAction*>("actionWriteExpenseNote");
    if(actCreate) {
        auto tv = expConts->findChild<QTableView*>("tableView");
        auto actRemove = new QAction("Hapus", tv);
        actRemove->setProperty("enableWithPerm", "ManageExpenses");
        connect(actRemove, &QAction::triggered,[this, tv]() {
            auto sm = tv->selectionModel();
            if(sm->hasSelection()) {
                auto ids = sm->selectedIndexes()[0].siblingAtColumn(0).data(Qt::EditRole).toLongLong();
                ExpenseItem ei(ids);
                auto yes = MessageHelper::question(nullptr, "Konfirmasi", QString("Yakin menghapus data pengeluaran ?"));
                if(yes == QMessageBox::Yes) {
                    if(ei.erase())
                        emit expenseRemoved();
                }
            }
        });
        tv->addAction(actCreate);
        tv->addAction(actRemove);
        
        // install editor pengeluaran
        connect(tv, &QTableView::doubleClicked, [this, tv, expConts](QModelIndex mi) {
            if(!UserPermissions::hasPermission(PermissionItem("ManageExpenses"))) {
                MessageHelper::warning(expConts, "Peringatan", "Anda tidak memiliki hak untuk mengubah data pengeluaran");
                return;
            }
            auto eed = new EditExpenseDialog(mi.siblingAtColumn(0).data(Qt::EditRole).toLongLong(), expConts);
            eed->setAttribute(Qt::WA_DeleteOnClose);
            connect(eed, &AddExpenseDialog::expenseModified, this, &ExpenseModule::expenseModified);
            eed->open();
        });
    }
    
    auto menuView = mw->menuBar()->findChild<QMenu*>("menuView");
    if(menuView) {
        auto act = expDock->toggleViewAction();
        menuView->addAction(act);
        act->setProperty("enableWithPerm", "Viewer");
    }
    expDock->hide();
}

void ExpenseModule::onWriteExpenseNote() {
    auto aed = new AddExpenseDialog(mainWindow);
    aed->setAttribute(Qt::WA_DeleteOnClose);
    connect(aed, SIGNAL(expenseAdded()), this, SIGNAL(expenseAdded()));
    aed->open();
}