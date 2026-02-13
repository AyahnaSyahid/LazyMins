#ifndef INVOICEPRINTER_H
#define INVOICEPRINTER_H

#include <QObject>

#include "invoicedatatype.h"
#include <QDialog>

class InvoicePrinter : public QObject
{
  Q_OBJECT
  public:
    static InvoicePrinter &instance();
    void drawInvoice(const PrintInvoiceParams& pip) const;
    QDialog *receiptPreview(qlonglong invoice_id, QWidget *parent=nullptr) const;

  private:
    explicit InvoicePrinter(QObject *parent=nullptr) : QObject(parent) {}
    ~InvoicePrinter() {}
};

#endif