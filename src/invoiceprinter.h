#ifndef INVOICEPRINTER_H
#define INVOICEPRINTER_H

#include <QObject>

class InvoicePrinter : public QObject
{
  Q_OBJECT
  public:
    explicit InvoicePrinter(QObject *parent) : QObject(parent) {}
    ~InvoicePrinter() {};
    void drawInvoice(int id);
};

#endif