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
  // Kelas ini hanya bertidak sebagai penerima input data bayar
  // parameter pembayaran diserahkan sepenuknya ke caller
  Q_OBJECT
  public:
    explicit PaymentDialog(QWidget * = nullptr);
    ~PaymentDialog();

    void setInvoiceId(int);     // lookup from database
    
    // setel nilai pembayaran total jika invoice belum dibuat
    void setInvoiceValue(int);  

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
    void resetModel();
    bool checkInput();
    
    Ui::PaymentDialog *ui;
    QSqlRecord m_invoiceRecord;
    QStandardItemModel *m_paymentModel;
    QSqlQueryModel     *m_akunModel;
};