#include "expensedialog.h"
#include "ui_expensedialog.h"

#include "src/customs/buttonguard.h"
#include "src/dialogs/basepickerdialog.h"

#include <QMessageBox>

namespace
{
  QString catQuery("SELECT id, nama, description FROM kategori_transaksi WHERE tipe = 'pengeluaran' AND is_active = 1");
  QString accQuery("SELECT id, nama, tipe, saldo FROM akun_transaksi WHERE is_active = 1");
}

ExpenseDialog::ExpenseDialog(QWidget *p) : ui(new Ui::ExpenseDialog), QDialog(p)
{
  ui->setupUi(this);
  ui->kategoriCombo->setQuery(catQuery);
  ui->kategoriCombo->boxViewAutoResize();
  ui->akunCombo->setQuery(accQuery);
  ui->akunCombo->boxViewAutoResize();
}

ExpenseDialog::~ExpenseDialog() { delete ui; }

QVariantMap ExpenseDialog::collectParams() const
{
  int catId = ui->kategoriCombo->currentIndex() < 0 ? -1 : ui->kategoriCombo->model()->index(ui->kategoriCombo->currentIndex(), 0).data().toInt();
  int accId = ui->akunCombo->currentIndex() < 0 ? -1 : ui->akunCombo->model()->index(ui->akunCombo->currentIndex(), 0).data().toInt();
  if (catId >= 1 && accId >= 1)
  {
    return {{"kategori_id", catId}, {"akun_id", accId}, {"jumlah", ui->spinBox->value()}, {"keterangan", ui->plainTextEdit->toPlainText()}};
  }
  return {};
}

void ExpenseDialog::on_pilihKategori_clicked()
{
  BasePickerDialog bpd("Pilih Kategori", this);
  if (!bpd.setupData(catQuery, {{0, "ID"}, {1, "Nama"}, {2, "Deskripsi"}}))
  {
    QMessageBox::warning(this, "Kesalahan Internal", "Tidak dapat membuka picker");
    return;
  }
  bpd.setHiddenColumns({0});
  bpd.adjustDialogSize();
  connect(&bpd, &BasePickerDialog::idPicked, [this](int id)
          {
    auto ixs = ui->kategoriCombo->model()->match(ui->kategoriCombo->model()->index(0, 0), Qt::DisplayRole, id, 1, Qt::MatchExactly);
    if (ixs.count()) 
      ui->kategoriCombo->setCurrentIndex(ixs.first().row()); });
  bpd.exec();
}

void ExpenseDialog::on_pilihAkun_clicked()
{
  BasePickerDialog bpd("Pilih Akun", this);
  if (!bpd.setupData(accQuery, {{0, "ID"}, {1, "Nama"}, {2, "Tipe"}, {3, "Saldo"}}))
  {
    QMessageBox::warning(this, "Kesalahan Internal", "Tidak dapat membuka picker");
    return;
  }
  bpd.setHiddenColumns({0});
  bpd.adjustDialogSize();
  connect(&bpd, &BasePickerDialog::idPicked, [this](int id)
          {
    auto ixs = ui->akunCombo->model()->match(ui->akunCombo->model()->index(0, 0), Qt::DisplayRole, id, 1, Qt::MatchExactly);
    if (ixs.count()) 
      ui->akunCombo->setCurrentIndex(ixs.first().row()); });
  bpd.exec();
}

void ExpenseDialog::on_simpanButton_clicked()
{
  ButtonGuard guard(ui->simpanButton);
}