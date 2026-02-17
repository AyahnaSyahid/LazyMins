#include "repaymentinputdialog.h"
#include "ui_repaymentinputdialog.h"

#include "../databaseinterface.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlRecord>

RepaymentInputDialog::RepaymentInputDialog(int invoice_id, QWidget *p)
: ui(new Ui::RepaymentInputDialog), m_invoice(invoice_id), m_dataLoaded(false), QDialog(p)
{
  ui->setupUi(this);
  auto optRecord = DatabaseInterface::instance().getInvoiceRecord(m_invoice);
  if (optRecord) {
    auto record = *optRecord;
    ui->totalBox->setValue(record.value("total").toInt());
    ui->terbayarBox->setValue(record.value("paid").toInt());
    ui->sisaBox->setValue(ui->totalBox->value() - ui->terbayarBox->value());
    ui->labelSisa->setText("Sisa");
    m_dataLoaded = true;
  }
};

RepaymentInputDialog::~RepaymentInputDialog() { delete ui; }

void RepaymentInputDialog::on_bayarBox_valueChanged(int va) {
  auto ss = ui->totalBox->value() - ui->terbayarBox->value() - va;
  if (ss < 0) {
    ui->labelSisa->setText("Kembalian");
    ui->sisaBox->setValue(-ss);
  } else {
    ui->labelSisa->setText("Sisa");
    ui->sisaBox->setValue(ss);
  }
};

void RepaymentInputDialog::on_dealButton_clicked() {
  int total = ui->totalBox->value(),
      terbayar = ui->terbayarBox->value(),
      bayar = ui->bayarBox->value();
  bool ok;
  QString admin = QInputDialog::getText(this, "Nama Admin diperlukan", "Penerima", QLineEdit::Normal, "", &ok);
  if (ok && !admin.isEmpty()) {
    auto optInvoiceRecord = DatabaseInterface::instance().getInvoiceRecord(m_invoice);
    if ( optInvoiceRecord ) {
      auto unpaid = total - terbayar;
      auto method = ui->methodLineEdit->text();
      ok = DatabaseInterface::instance().savePayment(*optInvoiceRecord, admin, bayar <= unpaid ? bayar : unpaid, method.isEmpty() ? "CASH" : method);
      if (ok) {
        accept();
        return ;
      }
    }
  }
  QMessageBox::information(this, "Error", "Tidak dapat melakukan repayment");
  reject();
}