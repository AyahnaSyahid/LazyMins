#ifndef InvoiceMaker_H
#define InvoiceMaker_H

#include <QObject>
#include <QWidget>
class InvoiceMaker : public QObject
{
  Q_OBJECT
public:
  explicit InvoiceMaker(QObject *parent = nullptr);

public slots:
  // popup invoice dialog preview
  void showInvoice(int invID, QWidget* parentWidget=nullptr);
};

#endif