#include "stockrefilldialog.h"
#include "ui_stockrefilldialog.h"
#include "src/managers/itemflowservice.h"

#include <QMessageBox>
#include <QTimer>

StockRefillDialog::StockRefillDialog(QWidget *parent):
  ui(new Ui::StockRefillDialog), QDialog(parent)
{
  ui->setupUi(this);
}

StockRefillDialog::~StockRefillDialog() { delete ui; }

bool StockRefillDialog::setProductId(int pid)
{
  m_productId = pid;
  auto opt_prod = productManager.getById(pid);
  if (!opt_prod.has_value()) {
    QMessageBox::warning(nullptr, "Error", QString("Produk dengan id %1 tidak ditemukan").arg(pid));
    return false;
  }
  
  auto prod = (*opt_prod);
  ui->dataSpinBox->setValue(prod.value("stock").toDouble());
  ui->labelSku->setText(prod.value("sku").toString());
  ui->labelNama->setText(prod.value("name").toString());
  ui->labelDeskripsi->setText(prod.value("description").toString());
  return true;
}

void StockRefillDialog::on_simpanButton_clicked()
{
  auto dari  = ui->buyFromLineEdit->text().trimmed(),
       notes = ui->notesEdit->toPlainText();
  qreal  qty = ui->qtySpinBox->value();
  
  QStringList err;
  if(dari.isEmpty()) err << "- Nama penjual tidak boleh kosong";
  if(notes.isEmpty()) err << "- Berikan keterangan spesifik yang bersangkutan dengan masuknya barang";
  int q = static_cast<int>(qty);
  if(q <= 0 ) err << "- Jumlah barang masuk seharusnya lebih dari 0";
  
  if (err.size() > 0) {
    QMessageBox::warning(this, "Input bermasalah", QString("Mohon untuk mengisi data sesuai dengan kriteria :\n%1").arg(err.join("\n")));
    return ;
  }
  
  ItemFlowService ifs;
  // Melakukan refill
  auto result = ifs.stockIn(m_productId, qty, dari, notes);
  if(!result) {
    QMessageBox::warning(this, "Kesalahan", QString("Pesan kesalahan:\n%1\n").arg(ifs.errorString()));
    return ;
  }
  accept();
}

