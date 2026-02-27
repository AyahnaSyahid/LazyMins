#include "konsumendialog.h"
#include "ui_konsumendialog.h"

#include "src/managers/konsumenmanager.h"

#include <QTimer>
#include <QSqlRecord>

KonsumenDialog::KonsumenDialog(QWidget *p):
ui(new Ui::KonsumenDialog), FormDialog(p)
{
  ui->setupUi(this);
  setupFields();
}

void KonsumenDialog::setupFields() {
  setFields({
      { ui->namaLineEdit,         "nama_lengkap" },
      { ui->emailLineEdit,        "email" },
      { ui->telpLineEdit,         "nomor_telp" },
      { ui->alamatLineEdit,       "alamat" },
      { ui->kotaLineEdit,         "kota" },
      { ui->kodePosLineEdit,      "kode_pos" },
      { ui->kodeKonsumenLineEdit, "customer_code" },
      { ui->nPWPLineEdit,         "npwp" },
      { ui->notesEdit,            "catatan" }
  });
}

void KonsumenDialog::onPrepareCreate() {
  // ui->tipeBox->setCurrentText("Individual");
  ui->priceLevelBox->setCurrentIndex(0);
}

void KonsumenDialog::onPrepareModify() {
  ui->activeCheck->setChecked(m_originalRecord.value("is_active").toBool());
  ui->tipeBox->setCurrentText(m_originalRecord.value("customer_type").toString());
  ui->priceLevelBox->setLevelID(m_originalRecord.value("price_level_id").toInt());
}

bool KonsumenDialog::onSave(const QVariantMap& data) {
  // append manual widgets not registered in setFields()
  QVariantMap full = data;
  full["is_active"] = ui->activeCheck->isChecked() ? 1 : 0;
  // full["is_active"] = ui->activeCheck->isChecked() ? 1 : 0;

  KonsumenManager km;
  if (isCreateMode()) {
    // INSERT INTO customers (...)
    // e.g: db.insertCustomer(full);
    auto opt = km.create(data);
    if(opt) return true;
  } else {
    // UPDATE customers SET ... WHERE id = originalRecord().value("id")
    int id = originalRecord().value("id").toInt();
    auto opt = km.update(id, data);
    if(opt) return true;
    qDebug() << "UPDATE id=" << id << full;
  }
  return false;
}

void KonsumenDialog::on_simpanButton_clicked() {
  accept();
}