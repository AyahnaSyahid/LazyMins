#ifndef INVOICEPRINTER_H
#define INVOICEPRINTER_H

#include <QObject>

#include "invoicedatatype.h"

class InvoicePrinter : public QObject
{
  Q_OBJECT
  public:
    static InvoicePrinter &instance();
    void drawInvoice(const PrintInvoiceParams& pip) const;

  private:
    explicit InvoicePrinter(QObject *parent=nullptr) : QObject(parent) {}
    ~InvoicePrinter() {}
};

#endif