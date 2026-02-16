#include "paymentinputdialog.h"
#include "ui_paymentinputdialog.h"
#include "../databaseinterface.h"
#include "receiptpreviewdialog.h"
#include "../invoiceprinter.h"
#include <QShowEvent>
#include <QMessageBox>


PaymentInputDialog::PaymentInputDialog(const InvoiceData& ida, QWidget *parent) :
  ui(new Ui::PaymentInputDialog),
  m_invoiceSaved(false),
  m_paymentSaved(false),
  m_ida(ida),
  m_record(),
  QDialog(parent)
{
  ui->setupUi(this);
  ui->totalBox->setValue(ida.total);
  on_bayarBox_valueChanged(0);
}

PaymentInputDialog::PaymentInputDialog(const QSqlRecord& rec, QWidget *parent) :
  ui(new Ui::PaymentInputDialog),
  m_invoiceSaved(false),
  m_paymentSaved(false),
  m_ida(),
  m_record(rec),
  QDialog(parent)
{
  ui->setupUi(this);
  ui->totalBox->setValue(rec.value("total").toInt());
  on_bayarBox_valueChanged(0);
}

PaymentInputDialog::~PaymentInputDialog() {
  delete ui;
}

void PaymentInputDialog::on_dealButton_clicked() {
  int bayar = ui->bayarBox->value();
  if(bayar <= 0) {
    QMessageBox::warning(this, "Peringatan", "Menyimpan data pembayaran dengan nilai <= 0 tidak diizinkan");
    return ;
  }
  auto &di = DatabaseInterface::instance();
  int invoice_id = 0;
  if (m_record.isEmpty()) {
    // add initial payment
    QSqlRecord rec;
    int total = m_ida.total;
    int minimum = total <= bayar ? total : bayar;
    if (!di.saveInvoiceAndPayment(m_ida, minimum, ui->methodLineEdit->text(), rec)) {
      QMessageBox::warning(this, "Peringatan", "Data Invoice dan Pembayaran gagal disimpan");
      return ;
    }
    invoice_id = rec.value("id").toInt();
  } else {
    // add another payment
    int total = m_record.value("total").toInt();
    int minimum = total <= bayar ? total : bayar;
    if (!di.savePayment(m_record, m_record.value("admin").toString(), minimum, ui->methodLineEdit->text())) {
      QMessageBox::warning(this, "Peringatan", "Data Pembayaran gagal disimpan");
      return ;
    }
    invoice_id = m_record.value("id").toInt();
  }
  auto &iptr = InvoicePrinter::instance();
  qDebug() << "Printing";
  iptr.drawInvoice(di.getPrintInvoiceParams(invoice_id));
  accept();
}

void PaymentInputDialog::showEvent(QShowEvent *event) {
  QDialog::showEvent(event);
  if(ui->bayarBox) {
    ui->bayarBox->setFocus(Qt::ActiveWindowFocusReason);
  }
}

void PaymentInputDialog::on_bayarBox_valueChanged(int v) {
  
  int r = m_record.isEmpty() ? m_ida.total - v : m_record.value("total").toInt();
  if (r < 0) {
    auto pal = ui->labelSisa->palette();
    pal.setColor(QPalette::WindowText, QColor(0, 180, 0));
    ui->labelSisa->setPalette(pal);
    ui->labelSisa->setText("Angsul");
    ui->sisaBox->setValue(r * -1);
  } else {
    auto pal = ui->labelSisa->palette();
    pal.setColor(QPalette::WindowText, QColor(255, 0, 0));
    ui->labelSisa->setPalette(pal);
    ui->labelSisa->setText("Kirang");
    ui->sisaBox->setValue(r);
  }
}