#include "productsmodule.h"
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include "addproductdialog.h"
#include "productsdockwidget.h"

ProductsModule::ProductsModule(QObject *parent)
: QObject(parent) {
    mainWindow = qobject_cast<QMainWindow*>(parent);
}

void ProductsModule::installMenu(QMainWindow* _mw) {
    auto mw = _mw == nullptr ? mainWindow : _mw;
    auto menuBar = mw->menuBar();
    auto proMen = menuBar->addMenu("Produk");
    proMen->setObjectName("menuProduct");
    proMen->setProperty("enableWithPerm", "Viewer");
    auto act = proMen->addAction("Baru");
    act->setObjectName("actionAddProduct");
    act->setProperty("enableWithPerm", "AddProduct");
    connect(act, &QAction::triggered, this, &ProductsModule::onAddProduct);
}

void ProductsModule::installDocker(QMainWindow* _mw) {
    auto pdw = new ProductsDockWidget(_mw);
    pdw->setProperty("enableWithPerm", "Viewer");
    _mw->addDockWidget(Qt::LeftDockWidgetArea, pdw);
    auto menuView = _mw->findChild<QMenu*>("menuView");
    if(menuView) {
        auto act = pdw->toggleViewAction();
        act->setProperty("enableWithPerm", "Viewer");
        menuView->addAction(pdw->toggleViewAction());
    }
    connect(this, &ProductsModule::productAdded, pdw, &ProductsDockWidget::refreshModel);
    connect(pdw, &ProductsDockWidget::productsUpdated, this, &ProductsModule::productAdded);
}

void ProductsModule::onAddProduct() {
    auto dia = new AddProductDialog(mainWindow);
    dia->setAttribute(Qt::WA_DeleteOnClose);
    connect(dia, &AddProductDialog::productAdded, this, &ProductsModule::productAdded);
    dia->open();
}

void ProductsModule::onRemoveProduct() {}
void ProductsModule::onEditProduct() {}
