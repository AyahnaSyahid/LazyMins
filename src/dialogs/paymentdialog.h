#pragma once

#include <QDialog>
#include <QSqlRecord>

namespace Ui {
  class PaymentDialog;
}

class QStandardItemModel;
class QSqlQueryModel;
class PaymentDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit PaymentDialog(QWidget * = nullptr);
    ~PaymentDialog();
    void setInvoiceId(int);     // lookup from database
    void setInvoiceValue(int);  // setel nilai pembayaran total
    int currentTRAkun() const;
    int currentInvoiceId() const;

  signals:
    void paymentGranted(const QVariantMap& param);
  
  private slots:
    void verboseAkunTRCombo();
    void on_okButton_clicked();

    // update data pembayaran
    void on_dibayarkanSpinBox_valueChanged(int);
    void on_jumlahUangSpinBox_valueChanged(int);
  
  protected:
    QVariantMap collect() const;
  
  private:
    bool checkInput();
    
    Ui::PaymentDialog *ui;
    QSqlRecord m_invoiceRecord;
    QStandardItemModel *m_paymentModel;
    QSqlQueryModel     *m_akunModel;
};