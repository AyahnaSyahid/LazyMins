#include "akuntransakiopnamedialog.h"
#include "ui_akuntransaksiopnamedialog.h"

#include "src/managers/akuntransaksimanager.h"

AkunTransaksiOpnameDialog::AkunTransaksiOpnameDialog(QWidget *parent)
    : QDialog(parent) , ui(new Ui::AkunTransasiOpnameDialog)
{
    ui->setupUi(this);
}

AkunTransaksiOpnameDialog::~AkunTransaksiOpnameDialog()
{
    delete ui;
}

bool AkunTransaksiOpnameDialog::prepareOpname(int akunId)
{
    AkunTransaksiManager mgr;
    auto optAcc = mgr.getById(akunId);
    if (!optAcc) return false;
    
    m_record = optAcc.value();
    ui->nameLabel->setText(m_record.value("name").toString());
    ui->currentBox->setValue(m_record.value("saldo").toInt());
    ui->realBox->setValue(ui->currentBox->value());
    ui->adjustBox->setValue(ui->currentBox->value() - ui->realBox->value());
    return true;
}

void AkunTransaksiOpnameDialog::on_realBox_valueChanged(int a) {
    ui->adjustBox->setValue(ui->currentBox->value() - a);
}

void AkunTransaksiOpnameDialog::on_simpanButton_clicked() {
    if (ui->notesEdit->toPlainText().simplified().isEmpty()) {
        QMessageBox::warning(this, "Berikan Catatan", "Opname / Adjustment harus disertai dengan alasannya");
        return ;
    }
    
}
