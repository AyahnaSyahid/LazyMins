#include "createfinishingservicedialog.h"
#include "ui_createfinishingservicedialog.h"

#include "src/managers/finishingservicemanager.h"
#include <QMessageBox>


CreateFinishingServiceDialog::CreateFinishingServiceDialog(QWidget *p)
: QDialog(p), ui(new Ui::CreateFinishingServiceDialog)
{
  ui->setupUi(this);
}

CreateFinishingServiceDialog::~CreateFinishingServiceDialog() { delete ui; }


void CreateFinishingServiceDialog::on_simpanButton_clicked() {
  ui->simpanButton->setEnabled(false);
  QString kode        = ui->kodeEdit->text().trimmed();
  QString name        = ui->nameEdit->text().trimmed();
  QString unit        = ui->unitEdit->currentText().trimmed();
  int     price       = ui->priceEdit->value();
  QString description = ui->descriptionEdit->toPlainText().trimmed();
  
  if ( kode.size() < 2 || name.size() < 3 || unit.isEmpty() ||
          price <= 0 || description.size() < 5 ) {
    QStringList kriteria {
      "Jumlah karakter Kode lebih dari 2 huruf",
      "Jumlah karakter Nama lebih dari 3 huruf",
      "Nama unit tidak kosong",
      "Harga lebih dari 0 (nol)",
      "Deskripsikan finishing dengan jelas"
    };
    QMessageBox::warning(this, "Input tidak diterima", "Pastikan input anda memenuhi kriteria:\n" + kriteria.join("\n- "));
    ui->simpanButton->setEnabled(true);
    return;
  }

  FinishingServiceManager fsm;
  auto result = fsm.create( {
    {"code",        kode},
    {"name",        name},
    {"unit",        unit},
    {"description", description},
    {"is_active",   1},
    {"created_at",  QDateTime::currentDateTimeUtc()},
    {"updated_at",  QDateTime::currentDateTimeUtc()},
    {"price_per_unit",       price},
  });

  if (result.has_value()) {
    accept();
  } else {
    QMessageBox::warning(this, "Input tidak diterima", "Pesan kesalahan:\n" + fsm.errorString());
  }
  ui->simpanButton->setEnabled(true);
}