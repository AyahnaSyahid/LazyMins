#ifndef InvoiceViewDialog_H
#define InvoiceViewDialog_H

namespace Ui {
    class InvoiceViewDialog;
}

#include <QDialog>
#include <QLocale>

class QSqlQueryModel;
class Database;
class InvoiceViewDialog : public QDialog {
    Q_OBJECT

public:
    explicit InvoiceViewDialog(int c_id, Database*, QWidget* =nullptr);
    ~InvoiceViewDialog();

private slots:
  void on_invoiceView_customContextMenuRequested(const QPoint&);
  void requestPayment(int inv_id);
  void reselectModel();

private:
    QSqlQueryModel* model;
    Ui::InvoiceViewDialog* ui;
    Database *db;
};

#endif