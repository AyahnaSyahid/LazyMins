#ifndef PAYMENTDATAEDITORDIALOG_H
#define PAYMENTDATAEDITORDIALOG_H

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
    explicit PaymentDataEditorDialog(const QList<QSqlRecord> &r, QWidget *parent) ;
    ~PaymentDataEditorDialog() ;
  
  private:
    QList<QSqlRecord> recs;
    QList<QSqlRecord> recs_update;
    QStandardItemModel *itemModel;
    Ui::PaymentDataEditorDialog *ui;
};

#endif

