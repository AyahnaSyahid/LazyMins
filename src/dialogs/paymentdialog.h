#pragma once

#include <QDialog>

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
    void setInvoiceId(int); // lookup from database
    int currentTRAkun() const;
    int currentInvoiceId const;

  signals:
    void paymentAccepted(const QVariantMap& param);
  
  private slots:
    void verboseAkunTRCombo();

  private:
    bool checkInput();
    
    
    Ui::PaymentDialog *ui;
    QSqlRecord m_invoiceRecord;
    QStandardItemModel *m_paymentModel;
    QSqlQueryModel     *m_akunModel;
};