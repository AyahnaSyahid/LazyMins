#include "invoicecomposerdialog.h"
#include "ui_invoicecomposerdialog.h"

#include "src/customs/invoicecomposerdelegate.h"
#include "src/models/invoicecomposermodel.h"
#include "src/dialogs/orderpickerdialog.h"
#include "src/dialogs/customerpickerdialog.h"
#include "src/managers/managers.h"

#include <QDate>
#include <QMenu>
#include <QAction>
#include <QSqlRecord>
#include <QSqlQueryModel>
#include <QMessageBox>

InvoiceComposerDialog::InvoiceComposerDialog(QWidget *p):
ui(new Ui::InvoiceComposerDialog), model(new InvoiceComposerModel(this)), QDialog(p)
{
  ui->setupUi(this);
  ui->orderListView->setModel(model);
  ui->orderListView->setItemDelegate(new InvoiceComposerDelegate(this));
  ui->dateEdit->setDate(QDate::currentDate());
  
  // init metode bayar
  ui->metodeBayar->setQuery(R"-(
    SELECT id, kode, tipe, nama_bank, nomor_rekening, atas_nama, description FROM akun_transaksi WHERE is_active = 1 ORDER BY kode ASC
  )-");
  auto isOnlyMethod = ui->metodeBayar->count() == 1;
  ui->metodeBayar->setCurrentIndex( isOnlyMethod ? 0 : -1);
  ui->metodeBayar->setDisabled( isOnlyMethod ? true : false);
  ui->metodeBayar->showColumn(0, false);
  ui->metodeBayar->boxViewAutoResize();
  
  this->addAction(ui->importOrdersAction);
  ui->orderListView->addAction(ui->removeSelectedOrdersAction);
  connect(this,                   &InvoiceComposerDialog::customerChanged, this, &InvoiceComposerDialog::onCustomerChanged);
  connect(model,                  &QAbstractItemModel::rowsInserted,       this, &InvoiceComposerDialog::uiSync);
  connect(ui->pajakSpinBox,       &QSpinBox::valueChanged,                 this, &InvoiceComposerDialog::uiSync);
  connect(ui->importOrdersAction, &QAction::triggered,                     this, &InvoiceComposerDialog::onImportOrder);
  
  ui->line_2->hide();
  ui->totalLabel_2->hide();
  ui->totalSpinBox_2->hide();

};

InvoiceComposerDialog::~InvoiceComposerDialog() { delete ui; }

bool InvoiceComposerDialog::loadInvoice(int iid) // setelan untuk mode edit
{
  return false;
}

void InvoiceComposerDialog::on_metodeBayar_currentIndexChanged(int i) {
  if(ui->metodeBayar->count() == 1) {
    ui->akunBank->setText("Kas Admin");
    // ui->simpanButton->setEnabled(false);
    return;
  }
  auto  m_model = ui->metodeBayar->model();
  ui->akunBank->setText(m_model->index(i, 0).data().toString());
  ui->akunBank->setToolTip(QString("%1\n%2\n%3").arg(
    m_model->index(i, 4).data().toString(),
    m_model->index(i, 5).data().toString(),
    m_model->index(i, 6).data().toString()));
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
  if(opd.availableCount() < 1) { 
    QMessageBox::information(this, "Semua Order Sudah Diimpor",
    "Semua order milik pelanggan ini sudah ada di invoice.");
    return ;
  }
  connect(&opd, &OrderPickerDialog::ordersPicked, this, &InvoiceComposerDialog::importOrders);
  opd.exec();
}

void InvoiceComposerDialog::on_pilihButton_clicked()
{
  CustomerPickerDialog kpd;
  kpd.setWindowFlag(Qt::FramelessWindowHint, true);
  kpd.setModelQuery(R"-(
    SELECT k.id,
           nama_lengkap,
           pl.id AS pl_id,
           level_name,
           nomor_telp
      FROM konsumen k
           JOIN price_levels pl ON k.price_level_id = pl.id
     WHERE EXISTS (
               SELECT 1
                 FROM orders o
                WHERE o.customer_id = k.id
                  AND o.invoice_id IS NULL )
  )-");
  if (!kpd.availableCustomers()) {
    QMessageBox::information(nullptr, "Selesai", "Tidak ditemukan konsumen yang memiliki order tanpa invoice");
    return ;
  }
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

void InvoiceComposerDialog::on_removeSelectedOrdersAction_triggered() {
  auto ixs = ui->orderListView->selectionModel()->selectedIndexes();
  if( !ixs.size() ) return;
  QSet<int> orderIds;
  for (auto const& ix : ixs) {
    orderIds << ix.data(InvoiceComposerModel::IdRole).toInt();
  }
  for(auto const& id : orderIds) model->removeOrder(id);
}

void InvoiceComposerDialog::on_orderListView_customContextMenuRequested(const QPoint& p)
{
  QMenu ctx;
  ctx.setToolTipsVisible(true);
  auto act = ctx.addAction("Import");
  connect(act, &QAction::triggered, this, &InvoiceComposerDialog::onImportOrder);
  auto ix = ui->orderListView->indexAt(p);
  if(ix.isValid()) {
    int id = ix.data(InvoiceComposerModel::IdRole).toInt();
    ctx.addSeparator();
    auto actDelete = ctx.addAction("Hapus");
    connect(actDelete, &QAction::triggered, ui->removeSelectedOrdersAction, &QAction::trigger);
  }
  ctx.exec(ui->orderListView->viewport()->mapToGlobal(p));
}

void InvoiceComposerDialog::onCustomerChanged() {
  // model->clearOrders(); 
  ui->labelNama->setText(m_customer_name);
  ui->phoneLineEdit->setText(m_customer_phone);
}

void InvoiceComposerDialog::uiSync()
{
  auto sub   = model->subtotal();
  auto disc  = model->discount();
  auto total = sub - disc;
  auto pajak = ui->pajakSpinBox->value();
  
  ui->subtotalLineEdit->setText(locale().toString(sub));
  ui->diskonTotalLineEdit->setText(locale().toString(disc));
  ui->totalSpinBox->setValue(total + pajak);
}

void InvoiceComposerDialog::refresh()              // reload data orders dari didatabase
{
  
}

void InvoiceComposerDialog::setCustomer(const QSqlRecord& rc)
{
    auto newId = rc.value("id").toInt();

    if (newId < 1) return;
    if (newId == m_customer_id) return;

    if (m_customer_id > 0 && model->rowCount() ) {
        auto confirm = QMessageBox::question(
            this,
            "Ganti Pelanggan?",
            "Mengganti pelanggan akan menghapus semua order yang sudah diimpor.\n"
            "Lanjutkan?",
            QMessageBox::Yes | QMessageBox::No
        );
        if (confirm == QMessageBox::No) return;
        model->clearOrders();
    }
    m_customer_id    = rc.value("id").toInt();
    m_customer_name  = rc.value("nama_lengkap").toString();
    m_customer_phone = rc.value("nomor_telp").toString();
    emit customerChanged();
}