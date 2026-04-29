#include "paymentdialog.h"
#include "ui_paymentdialog.h"

#include <QMessageBox>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSqlQueryModel>
#include <QStandardItem>
#include <QTimeZone>
#include <QHeaderView>
#include "src/managers/managers.h"

namespace {
  class PaymentDialogDelegate : public QStyledItemDelegate {
  public:
    PaymentDialogDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
  protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex& index) const override {
      QStyledItemDelegate::initStyleOption(option, index);
      switch (index.column())
      {
        case 0:
          option->displayAlignment = Qt::AlignCenter;
          break;
        case 1:
          option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
          break;
      }
      if (index.row() == index.model()->rowCount() - 1) {
        option->font.setBold(true);
        option->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
      }
    }
  };
}

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
  m_paymentModel->setHorizontalHeaderLabels({"Tanggal", "Nilai"});
  ui->tableView->setModel(m_paymentModel);
  ui->tableView->horizontalHeader()->setStretchLastSection(true);
  ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableView->setAlternatingRowColors(true);
  ui->tableView->setItemDelegate(new PaymentDialogDelegate(this));
  ui->tableView->verticalHeader()->hide();
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
    sumVal += value;
    auto tanggalItem = new QStandardItem(tanggal.toString("dd MMMM yyyy"));
    auto valueItem   = new QStandardItem(QString("%L1").arg(value));
    m_paymentModel->appendRow({ tanggalItem, valueItem });
  }
  auto rem = m_invoiceRecord.value("remaining_amount").toInt();
  auto sumItem = new QStandardItem(QString("Terbayar : %L1").arg(sumVal));
  m_paymentModel->appendRow(sumItem);
  ui->tableView->setSpan(m_paymentModel->rowCount() -1, 0, 1, 2);
  ui->tableView->resizeColumnsToContents();
  ui->belumBayarSpinBox->setValue(rem);
  ui->sisaSpinBox->setValue(rem);
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
    { "payment_date",      QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss") },
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