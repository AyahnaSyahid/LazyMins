#ifndef PAYMENTDATAEDITORDIALOG_H
#define PAYMENTDATAEDITORDIALOG_H

#include "../invoicedatatype.h"
namespace Ui {
  class PaymentDataEditorDialog;
}

#include <QDialog>
#include <QStandardItemModel>
#include <QSqlRecord>
#include <QStandardItem>

class PaymentDataEditorDialog : public QDialog
{
  public:
    explicit PaymentDataEditorDialog(int required, const QList<QSqlRecord> &r, QWidget *parent) ;
    ~PaymentDataEditorDialog() ;
    
    QList<PaymentData> getPaymentsData() const;
    void on_tableView_customContextMenuRequested(const QPoint &p);
  private:
    QList<QSqlRecord> recs;
    QList<QSqlRecord> recs_update;
    QStandardItemModel *itemModel;
    Ui::PaymentDataEditorDialog *ui;
    int m_req;
};

#endif

