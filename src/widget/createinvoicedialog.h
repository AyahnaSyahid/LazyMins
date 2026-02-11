#ifndef CREATEINVOICEDIALOG_H
#define CREATEINVOICEDIALOG_H

#include <QDialog>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QSqlQueryModel>

#include "../invoicedatatype.h"

namespace Ui {
  class CreateInvoiceDialog;
};

class CreateInvoiceDialog : public QDialog {
  Q_OBJECT
  public:
    static void RegisterMetaType();
    explicit CreateInvoiceDialog(QWidget *parent=nullptr);
    InvoiceData getInvoiceData() const;

    ~CreateInvoiceDialog();
  
  private slots:
    void on_tableActionInsert_triggered();
    void on_tableActionDelete_triggered();
    void on_simpanButton_clicked();
    void on_bayarButton_clicked();
    void inputDialogAccepted();
    void onNotaSaveDone(bool);
    void onPaymentDialogFinished(int);
    void updateTotalPrice();
    void resetUi();

  protected:
    void resizeEvent(QResizeEvent *re) override;
    void showEvent(QShowEvent *se) override;
  
  signals:
    void saveNotaRequest(const InvoiceData &ida);
    void invoiceSaved();
    void paymentSaved();
  
  private:
    int totalPrice() const;
    bool verifyInvoiceData(const InvoiceData& ida, QString &err) const;
    Ui::CreateInvoiceDialog *ui;
    QStandardItemModel *notaModel;
    QSqlQueryModel *konsumenModel;
    QSqlQueryModel *adminModel;
};
#endif