#include "depositdialog.h"
#include "ui_depositdialog.h"

#include "src/managers/akuntransaksimanager.h"
#include "src/customs/buttonguard.h"
#include <QMessageBox>

DepositDialog::DepositDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DepositDialog)
{
    ui->setupUi(this);
}

DepositDialog::~DepositDialog()
{
    delete ui;
}

void DepositDialog::prepareModify(const QSqlRecord& rec)
{
    m_record = rec;
    ui->nameLabel->setText(rec.value("nama").toString());
    ui->balanceLabel->setText(locale().toString(rec.value("saldo").toInt()));
    ui->jumlahSpinBox->setMaximum(rec.value("saldo").toInt());
}

void DepositDialog::on_simpanButton_clicked()
{
    ButtonGuard guard(ui->simpanButton);
    if (ui->jumlahSpinBox->value() == 0) {
        QMessageBox::warning(this, "Jumlah Deposit", "Jumlah deposit tidak boleh nol (0)");
        return;
    }
    if(ui->notesEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Catatan", "Catatan tidak boleh kosong");
        return;
    }

    int jumlah = ui->tipeCombo->currentIndex() == 0 ? -ui->jumlahSpinBox->value() : ui->jumlahSpinBox->value();

    AkunTransaksiManager mgr;
    if (!mgr.deposit(m_record.value("id").toInt(), jumlah, ui->notesEdit->toPlainText())) {
        QMessageBox::warning(this, "Gagal melakukan Deposit", mgr.errorString());
        return;
    }
    accept();
}