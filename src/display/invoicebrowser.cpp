#include "invoicebrowser.h"
#include "ui_dataviewer.h"

#include <QMenu>
#include <QAction>

InvoiceBrowser::InvoiceBrowser(QWidget *parent) : DataViewer(parent)
{
    setQueryArgs("SELECT * FROM invoices");
    Ui()->dataView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(Ui()->dataView, &QAbstractItemView::customContextMenuRequested, this, &InvoiceBrowser::on_dataView_customContextMenuRequested);
}

void InvoiceBrowser::on_dataView_customContextMenuRequested(const QPoint& p){
    QMenu ctx;
    ctx.setToolTipsVisible(true);

    auto printMenu = ctx.addMenu("Print");
    auto serial = printMenu->addAction("Print to Thermal");
    serial->setEnabled(false);

    auto ix = Ui()->dataView->indexAt(p);
    if(ix.isValid()) {
        serial->setEnabled(true);
        int id = ix.siblingAtColumn(0).data().toInt();
        connect(serial, &QAction::triggered, [this, id](){
            emit this->serialPrintRequested(id);
        });
    }
    ctx.exec(Ui()->dataView->viewport()->mapToGlobal(p));
}