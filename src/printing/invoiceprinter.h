#ifndef InvoicePrinter_H
#define InvoicePrinter_H

#include <QObject>

class InvoicePrinter : public QObject
{
  Q_OBJECT

public:
  explicit InvoicePrinter(QObject * = nullptr);
  ~InvoicePrinter();

public slots:
  void printInvoice(int id);
  void openConfig();
  void openPreviewDialog(int id);

signals:
  void printingDone(int);

};

QImage createBarcode(const QString& text, int width_px, int height_px);

#endif