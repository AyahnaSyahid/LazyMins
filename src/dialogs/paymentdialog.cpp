#include "paymentdialog.h"
#include "ui_paymentdialog.h"

#include <QMessageBox>
#include <QStandardItemModel>
#include <QSqlQueryModel>
#include <QStandardItem>
#include <QStandardItem>
#include "src/managers/managers.h"

PaymentDialog::PaymentDialog(QWidget *p) : 
ui(new Ui::PaymentDialog), m_paymentModel(new QStandardItemModel(this)), QDialog(p) 
{
  ui->setupUi(this);
    // init metode bayar
  ui->akunTransaksiComboBox->setQuery(R"-(
    SELECT id, kode, nama, nama_bank, nomor_rekening, atas_nama, description FROM akun_transaksi WHERE is_active = 1 ORDER BY kode ASC
  )-");

  auto isOnlyMethod = ui->metodeBayar->count() == 1;
  ui->akunTransaksiComboBox->setCurrentTEXT( isOnlyMethod ? 0 : -1);
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
  
  PaymentManager paymentManager;
  m_paymentModel->clear();
  auto payment_records = paymentManager.getWhere("invoice_id = :iid", {{"iid", m_invoiceRecord.value("id")}});
  auto sumVal = 0;
  for (auto const& pr : payment_records) {
    auto tanggal = pr.value("payment_date").toDateTime();
    auto value   = pr.value("amount").toInt();
    sumVal      += value;
    tanggal.setTimeZone(QTimeZone::LocalTime);
    auto tanggalItem = new QStandardItem(tanggal.toDate().toString("dd MMMM yyyy"));
    auto valueItem   = new QStandardItem(QString("%L1").arg(value));
    m_paymentModel->insertRow(m_paymentModel->rowCount(), { tanggalItem, valueItem });
  }
  m_paymentModel->insertRow(m_paymentModel->rowCount(), { new QStandardItem("Terbayar"), new QStandardItem(QString("%L1").arg(sumVal))} );
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
  if (!ui->akunTransaksiComboBox->isEnabled()) return -1;
  if (ui->akunTransaksiComboBox->currentText().isEmpty()) return -1;
  auto akunModel = qobject_cast<QSqlQueryModel*>(ui->akunTransaksiComboBox->model());
  if(!akunModel) return -1;
  return akunModel->data(akunModel->index(ui->akunTransaksiComboBox->currentIndex(), 0)).toInt());
}

int PaymentDialog::currentInvoiceId() const {
  auto rv = m_invoiceRecord.value("id");
  return rv.isValid() ? rv.toInt() : -1;
}