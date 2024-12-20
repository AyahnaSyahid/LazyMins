#include "konsumenview.h"
#include "customermodel.h"
#include "loginnotifier.h"
#include "userpermissions.h"
#include <QMenu>
#include <QAction>

KonsumenView::KonsumenView(QWidget* parent)
  : cmodel(new CustomerModel(this)), QTableView(parent)
{
    setModel(cmodel);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customContextMenu(QPoint)));
}

KonsumenView::~KonsumenView()
{
    delete cmodel;
}

void KonsumenView::setModelFilter(const QString& s)
{
    cmodel->setFilter(s);
}

void KonsumenView::customContextMenu(const QPoint& p)
{
    QMenu contextMenu;
    auto sm = selectionModel();
    if(!sm->hasSelection()) return;
    auto sr = selectionModel()->selectedRows(0).at(0);
    auto actOrder = contextMenu.addAction("Order");
    
    // just in case users without allowed permmission arrive here
    if(!UserPermissions::hasPermission(LoginNotifier::currentUser().user_id, PermissionItem("AddOrder"))) {
        actOrder->setEnabled(false);
        actOrder->setToolTip("Anda tidak memiliki hak akses untuk membuat order");
    }
    
    auto pic = contextMenu.exec(viewport()->mapToGlobal(p));
    if(pic) {
        if(pic == actOrder) {
            emit manageCustomerOrder(sr.data(Qt::EditRole));
        }
    }
}

void KonsumenView::modelSelect() {
    cmodel->select();
}