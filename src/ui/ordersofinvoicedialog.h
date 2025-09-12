#ifndef ORDERSOFINVOICEDIALOG_H
#define ORDERSOFINVOICEDIALOG_H

#include <QDialog>
#include "models/ordersofinvoicemodel.h"
#include "database.h"

namespace Ui {
  class OrdersOfInvoiceDialog;
}

class OrdersOfInvoiceDialog : public QDialog {
 Q_OBJECT
 public:
  OrdersOfInvoiceDialog(int inv, Database *database, QWidget *parent = nullptr);
  ~OrdersOfInvoiceDialog();
  
 private:
  Ui::OrdersOfInvoiceDialog *ui;
  OrdersOfInvoiceModel *mod;
  Database *db;
};

#endif // ORDERSOFINVOICEDIALOG_H