#include "editinvoicedialog.h"
#include "ui_createinvoicedialog.h"
#include "paymentdataeditordialog.h"

#include "../databaseinterface.h"
#include <QMessageBox>

EditInvoiceDialog::EditInvoiceDialog(int invoice_id, QWidget *p) 
  : targetInvoice(invoice_id), CreateInvoiceDialog(p)
{
  auto old = DatabaseInterface::instance().getInvoiceData(invoice_id);
  m_old = *old;
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
  
  if (!verifyInvoiceData(ida, verifyError)) {
    QMessageBox::information(this, "Tidak dapat menyimpan perubahan", verifyError);
    return;
  }
  
  if (ida == m_old) {
    QMessageBox::information(this, "Status Perubahan data", 
                            "Tidak ditemukan perubahan dalam data\nOperasi dibatalkan");
    accept();
    return;
  }
  
  // Payment check
  auto paymentRecords = DatabaseInterface::instance().getPaymentRecords(targetInvoice);
  
  // Case 1: No existing payments - simple update
  if (paymentRecords.count() == 0) {
    auto updateOk = DatabaseInterface::instance().updateInvoice(
        targetInvoice, ida, {}, 0, 
        DatabaseInterface::UpdateInvoiceStrategy::UpdateInvoiceSimple);
    if (updateOk) {
      accept();
      return;
    }
    QMessageBox::information(this, "Operasi gagal", 
                            "Update invoice tidak berjalan dengan baik.\nERR [EIDUIS]");
    return;
  }
  
  // Case 2: Has existing payments - need to decide what to do
  int total_paid = 0;
  for (const auto &rec : paymentRecords) {
    total_paid += rec.value("amount").toInt();
  }
  
  auto ask = QMessageBox::question(this, "Atur data pembayaran", 
                                   "Invoice ini terkait dengan beberapa pembayaran.\n"
                                   "Apakah anda akan menghapus semua data pembayaran?\n"
                                   "(anda dapat mengaturnya nanti seperti mencatat pembayaran invoice baru)");
  
  if (ask == QMessageBox::Yes) {
    // User wants to remove all payments
    auto updateOk = DatabaseInterface::instance().updateInvoice(
        targetInvoice, ida, {}, 0, 
        DatabaseInterface::UpdateInvoiceStrategy::UpdateAndRemovePayments);
    if (updateOk) {
      accept();
      return;
    }
    QMessageBox::information(this, "Operasi gagal", 
                            "Update invoice tidak berjalan dengan baik.\nERR [EIDUARP]");
    return;
  }
  
  // User wants to keep payments - need to adjust them
  if (total_paid > ida.total) {
    // Customer overpaid - offer cashback
    int cashback_amount = total_paid - ida.total;
    QString cashBackValue = locale().toCurrencyString(cashback_amount);
    
    auto ask2 = QMessageBox::question(this, "Memerlukan penyesuaian", 
                                     "Pembayaran yang telah dilakukan lebih besar dari nilai penjualan yang baru.\n"
                                     "Diperlukan pengembalian uang (cashback) sebesar " + cashBackValue + "\n"
                                     "Apakah anda ingin mencatat cashback ini?");
    
    if (ask2 == QMessageBox::Yes) {
      // Record cashback
      auto updateOk = DatabaseInterface::instance().updateInvoice(
          targetInvoice, ida, {}, cashback_amount, 
          DatabaseInterface::UpdateInvoiceStrategy::UpdateAndMakeCashBack);
      if (updateOk) {
        accept();
        return;
      }
      QMessageBox::information(this, "Operasi gagal", 
                              "Update invoice tidak berjalan dengan baik.\nERR [EIDUAMC]");
      return;
    }
    // User declined cashback - let them edit payments manually
  } else if (total_paid < ida.total) {
    // Customer underpaid - inform and let them adjust
    int shortfall = ida.total - total_paid;
    QString shortfallValue = locale().toCurrencyString(shortfall);
    
    QMessageBox::information(this, "Memerlukan penyesuaian", 
                            "Pembayaran yang telah dilakukan kurang dari nilai penjualan yang baru.\n"
                            "Kekurangan: " + shortfallValue + "\n"
                            "Silakan sesuaikan data pembayaran.");
  }
  // If total_paid == ida.total, payments are balanced - just need to update
  
  // Open payment editor for manual adjustment
  PaymentDataEditorDialog pde(ida.total, paymentRecords, this);
  if (pde.exec() == QDialog::Rejected) {
    return;
  }
  
  // Get edited payment data and update
  QList<PaymentData> pd = pde.getPaymentsData();
  auto updateOk = DatabaseInterface::instance().updateInvoice(
      targetInvoice, ida, pd, 0, 
      DatabaseInterface::UpdateInvoiceStrategy::UpdateInvoiceAndPayments);
  
  if (updateOk) {
    accept();
    return;
  }
  
  QMessageBox::information(this, "Operasi gagal", 
                          "Update invoice tidak berjalan dengan baik.\nERR [EIDUIAP]");
}