#include "paymentdataeditordialog.h"
#include "ui_paymentdataeditordialog.h"

#include <QDate>

PaymentDataEditorDialog::PaymentDataEditorDialog(const QList<QSqlRecord> &_r, QWidget *p):
  recs(_r),
  recs_update {}, 
  itemModel(new QStandardItemModel), 
  ui(new Ui::PaymentDataEditorDialog), 
  QDialog(p)
{
  ui->setupUi(this);
  for(const auto &r : recs) {
    QList<QStandardItem*> row;
    auto admin = new QStandardItem();
    admin->setData(r.value("admin"), Qt::EditRole);
    admin->setEditable(true);
    auto amount = new QStandardItem();
    amount->setData(r.value("amount"), Qt::EditRole);
    amount->setEditable(true);
    auto method = new QStandardItem();
    method->setData(r.value("method"), Qt::EditRole);
    method->setEditable(true);
    auto paytime = new QStandardItem();
    paytime->setData(r.value("payment_time").toDate(), Qt::EditRole);
    paytime->setEditable(true);
    row << admin << amount << method << paytime;
    itemModel->appendRow(row);
  }
  ui->tableView->setModel(itemModel);
}

PaymentDataEditorDialog::~PaymentDataEditorDialog() { delete ui; }