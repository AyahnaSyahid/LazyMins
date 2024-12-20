#include "editrepaymentdialog.h"
#include "../ui/ui_editrepaymentdialog.h"
#include "helper.h"

#include <QPushButton>
#include <QModelIndex>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QtDebug>


EditRepaymentDialog::EditRepaymentDialog(const QSqlRecord& rec, QWidget* parent)
  : ui(new Ui::EditRepaymentDialog), r(rec), QDialog(parent)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Ubah");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Batal");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
    setFixedSize(size());
    int t = rec.value("cash").toInt();
    if(t) {
        ui->label->setText("Cash");
        ui->checkBox->setChecked(false);
    } else {
        t = rec.value("trf").toInt();
        ui->label->setText("Transfer");
        ui->checkBox->setChecked(true);
    }
    ui->label_2->setText(locale().toCurrencyString(t, QString("Rp. ")));
    ui->spinBox->setValue(t);
}

EditRepaymentDialog::~EditRepaymentDialog()
{
    delete ui;
}

void EditRepaymentDialog::on_buttonBox_accepted()
{
    QSqlQuery q;
    q.prepare("UPDATE repayment SET (cash, trf, mtime) = (?, ?, ?) WHERE id = ?");
    q.addBindValue(ui->checkBox->isChecked() ? 0 : ui->spinBox->value());
    q.addBindValue(ui->checkBox->isChecked() ? ui->spinBox->value() : 0);
    q.addBindValue(StringHelper::currentDateTimeString());
    q.addBindValue(r.value("id"));
    q.exec();
    if(q.lastError().isValid()) {
        qDebug() << q.lastError().text();
    }
    accept();
}