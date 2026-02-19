#include "pencatatpengeluaran.h"
#include "ui_pencatatpengeluaran.h"
#include "src/databaseinterface.h"
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QComboBox>

PencatatPengeluaran::PencatatPengeluaran(QWidget *p) :
  m_tipe("Pengeluaran"), ui(new Ui::PencatatPengeluaran), QDialog(p)
{
  ui->setupUi(this);
  connect(ui->adminEdit, &QLineEdit::textChanged, [this](QString txt) { m_admin = txt; });
  connect(ui->tipeCombo, &QComboBox::currentTextChanged, [this](QString txt) { m_tipe = txt; });
  connect(ui->amountSpin, &QSpinBox::valueChanged, [this](int w) { m_amount = w; });
  connect(ui->detailText, &QPlainTextEdit::textChanged, [this]() { m_detail = ui->detailText->toPlainText(); });
}

PencatatPengeluaran::~PencatatPengeluaran() { delete ui; }

void PencatatPengeluaran::on_simpanButton_clicked() {
  auto drawMessage = [this](QString txt) {
    QMessageBox::warning(this, "Peringatan", txt);
  };
  
  if (m_admin.isEmpty()) {
    drawMessage("Nama Admin Penerima harus diisi.");
    return;
  }
  
  if (m_amount <= 0) {
    drawMessage("Jumlah uang harus diisi");
    return ;
  }
  
  if (m_detail.isEmpty()) {
    drawMessage("Detail text harus diisi sebagai pengingat transaksi");
    return;
  }
  
  if (DatabaseInterface::instance().recordCashFlow(m_admin, m_tipe.toLower(), m_amount, m_detail)) {
    accept();
  }
  return ;
}