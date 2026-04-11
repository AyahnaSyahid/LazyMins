#include "invoicecomposerdialog.h"
#include "ui_invoicecomposerdialog.h"

#include "src/customs/invoicecomposerdelegate.h"
#include "src/models/invoicecomposermodel.h"
#include "src/dialogs/orderpickerdialog.h"
#include "src/dialogs/paymentdialog.h"
#include "src/dialogs/customerpickerdialog.h"
#include "src/managers/managers.h"
#include "src/managers/helpers.h"

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
    SELECT id, kode, nama, nama_bank, nomor_rekening, atas_nama, description FROM akun_transaksi WHERE is_active = 1 ORDER BY kode ASC
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
  connect(ui->phoneLineEdit, &QLineEdit::textChanged, [this](const QString& txt) { m_customer_phone = txt; });
  
  ui->frame_3->hide();
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
  ui->akunBank->setText(m_model->index(i, 2).data().toString());
  ui->akunBank->setToolTip(QString("%1\n%2\n%3\n%4").arg(
    m_model->index(i, 3).data().toString(),
    m_model->index(i, 4).data().toString(),
    m_model->index(i, 5).data().toString(),
    m_model->index(i, 6).data().toString()).simplified());
}

void InvoiceComposerDialog::makePayment() {
  PaymentDialog pd(this);
  pd.setInvoiceValue(ui->totalSpinBox->value());
  connect(&pd, &PaymentDialog::paymentGranted, this, &InvoiceComposerDialog::handlePaymentGranted);
  connect(&pd, &QDialog::rejected, this, &InvoiceComposerDialog::handlePaymentRejected);
  pd.exec();
}

bool InvoiceComposerDialog::makeInvoice() {
  auto res = DBOperationHelper::createInvoiceForOrders(params(), model->imported());
  if(!res.ok) {
    QMessageBox::warning(this, "Gagal membuat Invoice", QString("Error :\n%1").arg(res.error));
    return false;
  }
  emit invoiceCreated(res.data["invoice_id"].toInt());
  return true;
}

void InvoiceComposerDialog::on_simpanButton_clicked()
{
  if (!checkInput()) return;
  if (makeInvoice()) {
    accept();
  }
}

void InvoiceComposerDialog::on_bayarButton_clicked()
{ 
  if (!checkInput()) return;
  makePayment();
}

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

// reload data orders dari didatabase
void InvoiceComposerDialog::refresh()
{
  auto imp = model->imported();
  model->clearOrders();
  for(auto const& i : imp) {
    model->insertOrder(i);
  }
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

bool InvoiceComposerDialog::checkInput()
{
  if (m_customer_id < 1) {
    QMessageBox::warning(this, "Periksa masukan", "Anda belum menentukan Konsumen");
    return false;
  }

  if (ui->phoneLineEdit->text().simplified().isEmpty()) {
    auto proceed = QMessageBox::question(this, "Nomor Telepon", "Apakah anda sengaja mengosongkan nomor telepon", QMessageBox::Yes | QMessageBox::No);
    if (proceed == QMessageBox::No) return false;
  }

  QStringList errs;
  if (ui->dateEdit->date() < QDate::currentDate()) {
    auto proceed = QMessageBox::question(this, "Tanggal Dibelakang", "Apakah anda sengaja membuat invoice dari tanggal sebelumnya ?", QMessageBox::Yes | QMessageBox::No);
    if (proceed == QMessageBox::No) return false;
  }
  return true;
}

QVariantMap InvoiceComposerDialog::params() const
{
  QVariantMap ret { 
    {"customer_id", m_customer_id},
    {"customer_name", m_customer_name},
    {"customer_phone", m_customer_phone},
    {"issue_date", ui->dateEdit->date().toString("yyyy-MM-dd")},
    {"tax_amount", ui->pajakSpinBox->value()} };
  
  if (!ui->plainTextEdit->toPlainText().simplified().isEmpty()) ret["notes"] = ui->plainTextEdit->toPlainText();
  return ret;
}

void InvoiceComposerDialog::handlePaymentGranted(const QVariantMap& vmap) {
  auto res = DBOperationHelper::createPaymentForOrders(params(), vmap, model->imported());
  if (res.ok) {
    emit invoiceCreated(res.data["invoice_id"].toInt());
    emit paymentCreated(res.data["payment_id"].toInt());
    accept();
    return;
  }
  QMessageBox::warning(this, "Operasi Gagal", res.error);
}

void InvoiceComposerDialog::handlePaymentRejected() {
  qWarning() << "Payment Rejected;";
}


