#include "paymentdialog.h"
#include "ui_paymentdialog.h"

#include <QMessageBox>
#include <QStandardItemModel>
#include <QSqlQueryModel>
#include <QStandardItem>
#include <QTimeZone>
#include "src/managers/managers.h"

PaymentDialog::PaymentDialog(QWidget *p) : 
ui(new Ui::PaymentDialog), m_paymentModel(new QStandardItemModel(this)), QDialog(p) 
{
  ui->setupUi(this);
    // init metode bayar
  ui->akunTransaksiComboBox->setQuery(R"-(
    SELECT id, kode, nama, nama_bank, nomor_rekening, atas_nama, description FROM akun_transaksi WHERE is_active = 1 ORDER BY kode ASC
  )-");

  auto isOnlyMethod = ui->akunTransaksiComboBox->count() == 1;

  // ui->akunTransaksiComboBox->setCurrentIndex( isOnlyMethod ? 0 : -1);
  ui->akunTransaksiComboBox->setDisabled( isOnlyMethod ? true : false);
  ui->akunTransaksiComboBox->showColumn(0, false);
  ui->akunTransaksiComboBox->boxViewAutoResize();
  connect(ui->akunTransaksiComboBox, &QComboBox::currentIndexChanged, this, &PaymentDialog::verboseAkunTRCombo);
  
  // m_akunModel
  m_akunModel = qobject_cast<QSqlQueryModel*>(ui->akunTransaksiComboBox->model());
  
  // m_paymentModel
  m_paymentModel->setColumnCount(2);
  m_paymentModel->setHorizontalHeaderLabels( {"Tanggal", "Nilai"} );  
}

PaymentDialog::~PaymentDialog() { delete ui; }

void PaymentDialog::setInvoiceId(int iid)
{
  InvoiceManager invoiceManager;
  auto opt_invoice = invoiceManager.getById(iid);
  if(!opt_invoice.has_value()) return;
  m_invoiceRecord = *opt_invoice;
  
  // set the label
  ui->noInvoiceLabel->setText(m_invoiceRecord.value("invoice_number").toString());
  
  PaymentManager paymentManager;
  m_paymentModel->clear();
  auto payment_records = paymentManager.getWhere("invoice_id = :iid", {{"iid", m_invoiceRecord.value("id")}});
  auto sumVal = 0;
  for (auto const& pr : payment_records) {
    auto tanggal = pr.value("payment_date").toDateTime().toLocalTime().date();
    auto value   = pr.value("amount").toInt();
    auto tanggalItem = new QStandardItem(tanggal.toString("dd MMMM yyyy"));
    auto valueItem   = new QStandardItem(QString("%L1").arg(value));
    m_paymentModel->insertRow(m_paymentModel->rowCount(), QList<QStandardItem*> { tanggalItem, valueItem });
  }
  auto rem = m_invoiceRecord.value("remaining_amount").toInt();
  auto sumItem = new QStandardItem(QString("%L1").arg(rem));
  m_paymentModel->insertRow(m_paymentModel->rowCount(), { new QStandardItem("Terbayar"), sumItem });
  ui->belumBayarSpinBox->setValue(rem);
}

void PaymentDialog::verboseAkunTRCombo()
{
  // set tool tip definition of akun_transaksi
  if (ui->akunTransaksiComboBox->currentIndex() == -1) ui->akunTransaksiComboBox->setToolTip("Belum disetel");
  QStringList desc;
  auto record = m_akunModel->record(ui->akunTransaksiComboBox->currentIndex());
  auto nama_bank = record.value("nama_bank");
  auto no_rek = record.value("nomor_rekening");
  auto an = record.value("atas_nama");
  if (nama_bank.isValid()) desc << QString("Bank : %1").arg(nama_bank.toString());
  if (no_rek.isValid()) desc << QString("Rekening : %1").arg(no_rek.toString());
  if (an.isValid()) desc << QString("Atas Nama : %1").arg(an.toString());
  
  if (desc.size()) ui->akunTransaksiComboBox->setToolTip(desc.join("\n"));
}

int PaymentDialog::currentTRAkun() const {
  if (ui->akunTransaksiComboBox->count() == 1) return 1;
  if (ui->akunTransaksiComboBox->currentText().isEmpty()) return -1;
  auto akunModel = qobject_cast<QSqlQueryModel*>(ui->akunTransaksiComboBox->model());
  if(!akunModel) return -1;
  return akunModel->index(ui->akunTransaksiComboBox->currentIndex(), 0).data().toInt();
}

int PaymentDialog::currentInvoiceId() const {
  auto rv = m_invoiceRecord.value("id");
  return rv.isValid() ? rv.toInt() : -1;
}

bool PaymentDialog::checkInput() {
  if (currentTRAkun() < 1) {
    QMessageBox::warning(this, "Input ditolak", "Anda belum menentukan akun transaksi");
    return false;
  }
  if (ui->dibayarkanSpinBox->value() == 0) {
    QMessageBox::warning(this, "Input ditolak", "Jumlah uang yang dibayarkan = 0");
    return false;
  }
  if (ui->dibayarkanSpinBox->value() > ui->belumBayarSpinBox->value()) {
    QMessageBox::warning(this, "Input ditolak", "Jumlah uang yang dibayarkan terlalu banyak");
    return false;
  }
  return true;
}

void PaymentDialog::setInvoiceValue(int tval)
{
  if (currentInvoiceId() > 0) return ; // blok jika invoice telah disetel
  ui->belumBayarSpinBox->setValue(tval);
  ui->sisaSpinBox->setValue(tval);
  ui->dibayarkanSpinBox->setMaximum(tval);
}

void PaymentDialog::on_dibayarkanSpinBox_valueChanged(int va)
{
  auto rem      = ui->belumBayarSpinBox->value();
  auto receive  = ui->jumlahUangSpinBox->value();
  
  ui->kembalianSpinBox->setValue(receive - va);
  ui->sisaSpinBox->setValue(rem - va);
}

void PaymentDialog::on_jumlahUangSpinBox_valueChanged(int va)
{
  auto rem      = ui->belumBayarSpinBox->value();

  if (va <= rem) {
    ui->dibayarkanSpinBox->setMaximum(va);
    ui->dibayarkanSpinBox->setValue(va);
    if ( va == rem) ui->kembalianSpinBox->setValue(0);
    return ;
  }
  
  ui->dibayarkanSpinBox->setMaximum(rem);
  ui->dibayarkanSpinBox->blockSignals(true);
  ui->dibayarkanSpinBox->setValue(rem);
  ui->dibayarkanSpinBox->blockSignals(false);
  auto willpaid = ui->dibayarkanSpinBox->value();
  
  ui->kembalianSpinBox->setValue(va - willpaid);
  ui->sisaSpinBox->setValue(rem - va);
}

void PaymentDialog::on_okButton_clicked() {
  if (!checkInput()) return ;
  emit paymentGranted(collect());
  accept();
}

QVariantMap PaymentDialog::collect() const {
  QVariantMap coll {
    { "akun_transaksi_id", currentTRAkun() },
    { "cash_received",     ui->jumlahUangSpinBox->value() },
    { "cash_change",       ui->kembalianSpinBox->value() },
    { "amount",            ui->dibayarkanSpinBox->value() },
    { "payment_date",      QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd") },
    { "verification_status", "pending" }
  };
  if (currentTRAkun() == 1) {
    coll["verification_status"] = "verified";
    coll["verified_at"]         = coll["payment_date"];
  }
  if ((!m_invoiceRecord.isEmpty()) && m_invoiceRecord.contains("id")) {
    coll["invoice_id"] = m_invoiceRecord.value("id");
  }
  return coll;
}