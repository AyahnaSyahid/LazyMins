#ifndef PAYMENTINPUTDIALOG_H
#define PAYMENTINPUTDIALOG_H

#include <QDialog>
#include <QSqlRecord>
#include "../invoicedatatype.h"

namespace Ui {
  class PaymentInputDialog;
}

class PaymentInputDialog : public QDialog
{
  Q_OBJECT
  
  public:
    explicit PaymentInputDialog(const InvoiceData& ida, QWidget *parent);
    explicit PaymentInputDialog(const QSqlRecord& record, QWidget *parent);
    ~PaymentInputDialog();
    
    inline bool invoiceSaved() const { return m_invoiceSaved; }
    inline bool paymentSaved() const { return m_paymentSaved; }
  
  private slots:
    void on_dealButton_clicked();
    void on_bayarBox_valueChanged(int);
  
  protected:
    void showEvent(QShowEvent *event) override;
  
  private:
    bool m_invoiceSaved;
    bool m_paymentSaved;
    InvoiceData m_ida;
    QSqlRecord m_record;
    Ui::PaymentInputDialog *ui;
};

#endif