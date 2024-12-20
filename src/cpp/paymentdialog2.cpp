#include "paymentdialog2.h"
#include "../ui/ui_paymentdialog.h"
#include "helper.h"
#include <QAbstractItemModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include <QtDebug>

PaymentDialog2::PaymentDialog2(QAbstractItemModel* mod, QWidget* parent)
  : PaymentDialog(mod, parent) {}
  
void PaymentDialog2::on_pushButton2_clicked()
{}

void PaymentDialog2::on_pushButton_clicked()
{}