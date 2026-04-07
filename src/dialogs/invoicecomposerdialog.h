#pragma once

namespace Ui {
  class InvoiceComposerDialog;
}

#include <QDialog>

class InvoiceComposerModel;
class QSqlRecord  ;
class InvoiceComposerDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit InvoiceComposerDialog (QWidget * = nullptr);
    ~InvoiceComposerDialog();
    
    bool loadInvoice(int iid); // untuk mode edit
  
  public slots:
    void uiSync();
    void refresh();             // ambil kembali data orders yang ada didatabase
    void setCustomer(const QSqlRecord&);

  private slots:
    void on_simpanButton_clicked();
    void on_bayarButton_clicked();
    void onImportOrder(); // buka dialog order picker
    void importOrders(const QList<int> &imported);
    void on_orderListView_customContextMenuRequested(const QPoint&);
    void on_pilihButton_clicked();

  private:
    Ui::InvoiceComposerDialog *ui;

    // data invoice
    int m_invoice_id = -1; // < 0 Menanadakan mode create
    InvoiceComposerModel *model;

    int m_customer_id = -1;
    QString m_customer_name = "";
    QString m_customer_phone = "";
};