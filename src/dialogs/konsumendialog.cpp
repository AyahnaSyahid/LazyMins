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

bool KonsumenDialog::onSave(const QVariantMap& data) {
  // append manual widgets not registered in setFields()
  QVariantMap full = data;
  full["status"] = ui->activeCheck->isChecked() ? 1 : 0;

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