#include "dynamicdialog.h"
#include <QSqlError>
InsertDialog::InsertDialog(QSqlTableModel *model, QWidget *parent) : QDialog(parent), model(model) {
        // Membuat layout utama
        QVBoxLayout *mainLayout = new QVBoxLayout;
        QFormLayout *formLayout = new QFormLayout;

        // Mendapatkan struktur tabel dari QSqlTableModel
        QSqlRecord record = model->record();

        // Iterasi setiap field dalam tabel
        for (int i = 0; i < record.count(); ++i) {
            QSqlField field = record.field(i);

            // Lewati field yang auto-generated (misalnya, order_id)
            if (field.isAutoValue()) {
                continue;
            }

            // Membuat label dengan nama field
            QString labelText = field.name();
            QLabel *label = new QLabel(labelText);

            // Membuat input widget (QLineEdit untuk semua tipe sederhana)
            QLineEdit *edit = new QLineEdit();

            // Jika field memiliki nilai default, isi input dengan nilai tersebut
            if (field.defaultValue().isValid()) {
                edit->setText(field.defaultValue().toString());
            }

            // Tambahkan ke form layout
            formLayout->addRow(label, edit);

            // Simpan input widget ke dalam map
            inputs[field.name()] = edit;

            // Tentukan apakah field wajib diisi (NOT NULL tanpa default)
            if (field.requiredStatus() == QSqlField::Required && field.defaultValue().isNull()) {
                requiredFields.append(field.name());
                label->setText(labelText + " *"); // Tandai field wajib dengan *
            }
        }

        // Tambahkan tombol OK
        QPushButton *okButton = new QPushButton("OK");
        connect(okButton, &QPushButton::clicked, this, &InsertDialog::onOkClicked);

        // Susun layout
        mainLayout->addLayout(formLayout);
        mainLayout->addWidget(okButton);
        setLayout(mainLayout);

        setWindowTitle("Insert New Order");
    }

void InsertDialog::onOkClicked() {
    // Membuat record baru untuk inser
    QSqlRecord newRecord = model->record();

    // Memeriksa dan mengisi nilai dari input pengguna
    for (const QString &fieldName : inputs.keys()) {
        QLineEdit *edit = inputs[fieldName];
        QString inputText = edit->text();

        if (!inputText.isEmpty()) {
            // Jika pengguna memasukkan nilai, set ke record
            newRecord.setValue(fieldName, inputText);
        } else if (requiredFields.contains(fieldName)) {
            // Jika field wajib tapi kosong, tampilkan pesan error
            QMessageBox::warning(this, "Error", "Harap isi semua field yang wajib diisi.");
            return;
        }
        // Jika field opsional dibiarkan kosong, tidak di-set, sehingga default atau NULL digunakan
    }

    // Masukkan record ke dalam model
    if (model->insertRecord(-1, newRecord)) {
        if (model->submitAll()) {
            accept(); // Tutup dialog jika sukses
        } else {
            QMessageBox::warning(this, "Error", "Gagal menyimpan data: " + model->lastError().text());
        }
    } else {
        QMessageBox::warning(this, "Error", "Gagal memasukkan record.");
    }
}