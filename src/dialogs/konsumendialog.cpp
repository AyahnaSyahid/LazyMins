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

KonsumenDialog::~KonsumenDialog() { delete ui; }

void KonsumenDialog::setupFields()
{
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

void KonsumenDialog::setupBoundFields()
{
    addBoundField("is_active",
    [this]{ return ui->activeCheck->isChecked() ? 1 : 0;},
    [this](const QVariant& v){ ui->activeCheck->setChecked(v.toBool()); }
    );

    addBoundField("customer_type", 
    [this]{ return ui->tipeBox->currentText();},
    [this](const QVariant& v){ ui->tipeBox->setCurrentText(v.toString());}
    );

    addBoundField("price_level_id",
    [this]{ return ui->priceLevelBox->currentId();},
    [this](const QVariant& v){ ui->priceLevelBox->setLevelID(v.toInt());}
    );
}

bool KonsumenDialog::onSave(const QVariantMap& changes)
{
    KonsumenManager km;

    if (isCreateMode()) {
        auto opt = km.create(changes);
        return opt.has_value();
    }
    else {
        int id = originalRecord().value("id").toInt();
        return km.update(id, changes);
    }
}
