#include "invoicecomposerdialog.h"
#include "ui_invoicecomposerdialog.h"

#include "src/customs/invoicecomposerdelegate.h"
#include "src/models/invoicecomposermodel.h"

#include <QDate>
#include <QMenu>
#include <QAction>

InvoiceComposerDialog::InvoiceComposerDialog(QWidget *p):
ui(new Ui::InvoiceComposerDialog), model(new InvoiceComposerModel(this)), QDialog(p)
{
  ui->setupUi(this);
  ui->orderListView->setModel(model);
  ui->orderListView->setItemDelegate(new InvoiceComposerDelegate(this));
  ui->dateEdit->setDate(QDate::currentDate());
};

InvoiceComposerDialog::~InvoiceComposerDialog() { delete ui; }

bool InvoiceComposerDialog::loadInvoice(int iid) // setelan untuk mode edit
{
  return false;
}

void InvoiceComposerDialog::on_simpanButton_clicked()
{}

void InvoiceComposerDialog::on_bayarButton_clicked()
{}

void InvoiceComposerDialog::onImportOrder() // buka dialog order picker
{}

void InvoiceComposerDialog::importOrders(const QList<int> &imported)
{
  QList<int> held_ids = model->imported();
  for(auto const& oid : imported) {
    if (!held_ids.contains(oid)) {
      model->insertOrder(oid);    
    }
  }
}

void InvoiceComposerDialog::on_orderListView_customContextMenuRequested(const QPoint& p)
{
  QMenu ctx;
  ctx.setToolTipsVisible(true);
  auto act = ctx.addAction("Import");
  connect(&act, &QAction::triggered, this, &InvoiceComposerDialog::onImportOrder);
  ctx.exec(ui->orderListView->viewport()->mapToGlobal(p));
}

void InvoiceComposerDialog::uiSync()
{
  if (!model->rowCount())
}

void InvoiceComposerDialog::refresh()              // reload data orders dari didatabase
{
  
}

void InvoiceComposerDialog::setCustomerId(int id) // setel konsumen
{
  
}