#include "invoicecomposerdialog.h"
#include "ui_invoicecomposerdialog.h"

#include "src/customs/invoicecomposerdelegate.h"
#include "src/models/invoicecomposermodel.h"
#include "src/dialogs/orderpickerdialog.h"
#include "src/dialogs/customerpickerdialog.h"

#include <QDate>
#include <QMenu>
#include <QAction>
#include <QSqlRecord>

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
{
  OrderPickerDialog opd(this);
  opd.setCustomerId(m_customer_id);
  opd.setFilterIds(model->imported());
  connect(&opd, &OrderPickerDialog::ordersPicked, this, &InvoiceComposerDialog::importOrders);
  opd.exec();
}

void InvoiceComposerDialog::on_pilihButton_clicked()
{
  CustomerPickerDialog kpd;
  kpd.setWindowFlag(Qt::FramelessWindowHint, true);
  connect(&kpd, &CustomerPickerDialog::customerPicked, this, &InvoiceComposerDialog::setCustomer);
  auto pos = ui->pilihButton->mapToGlobal(ui->pilihButton->rect().topRight());
  kpd.move(pos);
  kpd.exec();
}

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
  connect(act, &QAction::triggered, this, &InvoiceComposerDialog::onImportOrder);
  ctx.exec(ui->orderListView->viewport()->mapToGlobal(p));
}

void InvoiceComposerDialog::uiSync()
{
  if (!model->rowCount()) return;
}

void InvoiceComposerDialog::refresh()              // reload data orders dari didatabase
{
  
}

void InvoiceComposerDialog::setCustomer(const QSqlRecord& rc) // setel konsumen
{
  m_customer_id = rc.value("id").toInt();
  m_customer_name = rc.value("nama_lengkap").toString();
  m_customer_phone = rc.value("nomor_telp").toString();
  
  ui->labelNama->setText(m_customer_name);
  ui->phoneLineEdit->setText(m_customer_phone);
}