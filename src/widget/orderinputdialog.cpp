#include "orderinputdialog.h"
#include "ui_orderinputdialog.h"

#include <QMessageBox>

OrderInputDialog::OrderInputDialog(QWidget *parent) :
  ui(new Ui::OrderInputDialog), 
  m_namaBarang (),
  m_qty (1),
  m_harga (0),
  QDialog(parent)
{
  ui->setupUi(this);
  connect(ui->spinBoxHarga, &QSpinBox::valueChanged, [this](int v) { m_harga = v; });
  connect(ui->spinBoxQty, &QSpinBox::valueChanged, [this](int v) { m_qty = v; });
  connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, [this]() { m_namaBarang = ui->plainTextEdit->toPlainText().trimmed(); });
}

OrderInputDialog::~OrderInputDialog() {
  delete ui;
}

void OrderInputDialog::accept() {
  if (m_namaBarang.isEmpty() || subTotal() == 0) {
    QMessageBox::warning(this, "Periksa Input", "Kesalahan input\nPeriksa kembali Nama Barang, Harga Satuan, dam Qty");
    return;
  }
  QDialog::accept();
}