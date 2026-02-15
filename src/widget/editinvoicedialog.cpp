#include "editinvoicedialog.h"
#include "ui_createinvoicedialog.h"

#include "../databaseinterface.h"

EditInvoiceDialog::EditInvoiceDialog(int invoice_id, QWidget *p) 
  : CreateInvoiceDialog(p) 
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
}
EditInvoiceDialog::~EditInvoiceDialog() {} 