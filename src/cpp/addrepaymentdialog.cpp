#include "addrepaymentdialog.h"
#include "ui_addrepaymentdialog.h"
#include "helper.h"

#include <QPushButton>

AddRepaymentDialog::AddRepaymentDialog(QWidget* parent)
  : ui(new Ui::AddRepaymentDialog), QDialog(parent)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Simpan");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Batal");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    ui->spinBox->addAction(ui->actionFillToMax);
    setFixedSize(size());
}

AddRepaymentDialog::~AddRepaymentDialog()
{
    delete ui;
}

void AddRepaymentDialog::setValue(int t)
{
    ui->label_2->setText(locale().toCurrencyString(t, "Rp. "));
    ui->spinBox->setMaximum(t);
}

void AddRepaymentDialog::accept()
{
    if(ui->spinBox->value() == 0) {
        MessageHelper::information(this, "Kesalahan", "Periksa jumlah masukan");
        return;
    }
    
    QDialog::accept();
}

int AddRepaymentDialog::value() const
{
    return ui->spinBox->value();
}

bool AddRepaymentDialog::isTransfer() const
{
    return ui->checkBox->isChecked();
}

void AddRepaymentDialog::on_actionFillToMax_triggered()
{
    ui->spinBox->setValue(ui->spinBox->maximum());
}

CashbackDialog::CashbackDialog(QWidget* parent)
  : AddRepaymentDialog(parent)
{
    setWindowTitle("Atur Pengembalian");
    ui->label->setText("Total");
    ui->label_3->setText("Kembalikan");
    ui->checkBox->hide();
}


void CashbackDialog::setValue(int v)
{
    AddRepaymentDialog::setValue(v);
    ui->spinBox->setValue(v);
}
