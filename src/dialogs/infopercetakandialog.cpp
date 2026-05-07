#include "infopercetakandialog.h"
#include "ui_infopercetakandialog.h"

#include "src/controllers/infopercetakan.h"
#include "src/customs/buttonguard.h"

#include <QMessageBox>


InfoPercetakanDialog::InfoPercetakanDialog(QWidget *parent) : QDialog(parent), ui(new Ui::InfoPercetakanDialog)
{
    ui->setupUi(this);
    InfoPercetakanController ic;
    InfoPercetakan info = ic.getInfoPercetakan();
    ui->nameEdit->setText(QString::fromStdString(info.nama));
    ui->telpEdit->setText(QString::fromStdString(info.telp));
    ui->emailEdit->setText(QString::fromStdString(info.email));
    ui->addressEdit->setPlainText(QString::fromStdString(info.alamat));
    connect(this, &InfoPercetakanDialog::trySave, this, &InfoPercetakanDialog::saveInfoPercetakan);
    connect(this, &InfoPercetakanDialog::saveSuccess, this, &InfoPercetakanDialog::onSaveSuccess);
    connect(this, &InfoPercetakanDialog::saveFailed, this, &InfoPercetakanDialog::onSaveFailed);
}

InfoPercetakanDialog::~InfoPercetakanDialog()
{
    delete ui;
}

void InfoPercetakanDialog::onSaveSuccess()
{
    QMessageBox::information(this, "Berhasil Menyimpan", "Data berhasil diperbarui");
    accept();
}

void InfoPercetakanDialog::onSaveFailed(const QString &msg)
{
    QMessageBox::warning(this, "Gagal Menyimpan", msg);
}

void InfoPercetakanDialog::saveInfoPercetakan(const InfoPercetakan &info)
{
    InfoPercetakanController ic;
    std::string error;
    if (!ic.saveInfoPercetakan(info, &error))
    {
        emit saveFailed(QString::fromStdString(error));
        return;
    }
    emit saveSuccess();
}

void InfoPercetakanDialog::on_simpanButton_clicked()
{
    ButtonGuard guard(ui->simpanButton);
    InfoPercetakanController ic;
    InfoPercetakan info;
    info.nama = ui->nameEdit->text().toStdString();
    info.telp = ui->telpEdit->text().toStdString();
    info.email = ui->emailEdit->text().toStdString();
    info.alamat = ui->addressEdit->toPlainText().toStdString();
    emit trySave(info);
}
