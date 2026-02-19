#ifndef PAYMENTDATAEDITORDIALOG_H
#define PAYMENTDATAEDITORDIALOG_H

#include "../invoicedatatype.h"
namespace Ui {
  class PaymentDataEditorDialog;
}

#include <QDialog>
#include <QSqlRecord>

#include "../models/advancedquerymodel.h"

class PaymentDataEditorDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit PaymentDataEditorDialog(int invoice_id, QWidget *p=nullptr) ;
    ~PaymentDataEditorDialog() ;

    QList<PaymentData> getPaymentsData() const;
  
  private slots:
    void on_tableView_customContextMenuRequested(const QPoint &p);
    void on_simpanButton_clicked();
    
  private:
    AdvancedQueryModel *itemModel;
    Ui::PaymentDataEditorDialog *ui;
    int m_id;
};

class PaymentDataEditorModel : public AdvancedQueryModel {
  public:
    PaymentDataEditorModel(QObject *p=nullptr) : AdvancedQueryModel(p) {}
};

#endif

