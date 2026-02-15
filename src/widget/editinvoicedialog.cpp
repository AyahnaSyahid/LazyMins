#include "editinvoicedialog.h"
#include "ui_createinvoicedialog.h"

#include "../databaseinterface.h"
#include <QMessageBox>

EditInvoiceDialog::EditInvoiceDialog(int invoice_id, QWidget *p) 
  : targetInvoice(invoice_id), CreateInvoiceDialog(p)
{
  m_old = DatabaseInterface::instance().getInvoiceData(invoice_id);
  ui->customerLineEdit->setText(m_old.customerName);
  ui->customerPhoneLineEdit->setText(m_old.customerPhone);
  ui->tanggalDateEdit->setDate(QDate::fromString(m_old.dateString, "yyyy-MM-dd"));
  notaModel->insertRows(0, m_old.itemList.count());
  for(int i=0; i<notaModel->rowCount(); ++i) {
    auto ix = notaModel->index(i, 0);
    auto il = m_old.itemList.at(i);
    notaModel->setData(ix, i + 1, Qt::EditRole);
    notaModel->setData(ix.siblingAtColumn(1), il.productName, Qt::EditRole);
    notaModel->setData(ix.siblingAtColumn(2), il.unitPrice, Qt::EditRole);
    notaModel->setData(ix.siblingAtColumn(3), il.unitQty, Qt::EditRole);
    notaModel->setData(ix.siblingAtColumn(4), il.subTotal, Qt::EditRole);
  }
  setWindowTitle("Edit Invoice Data");
  ui->bayarButton->hide();
}

EditInvoiceDialog::~EditInvoiceDialog()
{}

void EditInvoiceDialog::on_simpanButton_clicked() {
  QString verifyError;
  auto ida = getInvoiceData();
  if ( !verifyInvoiceData(ida, verifyError) )  {
    QMessageBox::information(this, "Tidak dapat menyimpan perubahan", verifyError);
    return ;
  }
  if (ida == m_old) {
    QMessageBox::information(this, "Status Perubahan data", "Tidak ditemukan perubahan dalam data\nOperasi dibatalkan");
    accept();
    return;
  }
  // payment check
  auto paymentRecords = DatabaseInterface::instance().getPaymentRecords();
  if (paymentRecords.count() > 0) {
    int paid = 0;
    for(const auto &rec : paymentRecords) {
      paid += rec.value("amount").toInt();
    }
    if (paid > ida.total) {
      
    }
  }
  
  if (DatabaseInterface::instance().updateInvoice(targetInvoice, ida)) {
    accept() ;
    return ;
  }
  QMessageBox::information(this, "Status Perubahan data", "Tidak dapa menyimpan perubahan dalam data\nOperasi dibatalkan [ERR EID45]");  
}