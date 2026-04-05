#include "priceleveldialog.h"
#include "ui_priceleveldialog.h"
#include "src/managers/managers.h"
#include <QMessageBox>

PriceLevelDialog::PriceLevelDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PriceLevelDialog),
    m_priceLevelId(-1),
    m_name(""),
    m_description("")
{
    ui->setupUi(this);
}

PriceLevelDialog::~PriceLevelDialog() 
{ 
    delete ui; 
}

bool PriceLevelDialog::setPriceLevelId(int pid) 
{
    PriceLevelManager plm;
    auto opt_level = plm.getById(pid);
    
    if (!opt_level.has_value()) {
        QMessageBox::critical(this, "Kesalahan", 
            QString("Data tidak ditemukan,\nLevel Harga dengan ID %1 tidak tersedia.").arg(pid));
        return false;
    }

    // Ambil data dari QSqlRecord
    m_priceLevelId = opt_level->value("id").toInt();
    m_name         = opt_level->value("name").toString();
    m_description  = opt_level->value("description").toString();

    // Set tampilan UI
    ui->nameLineEdit->setText(m_name);
    ui->plainTextEdit->setPlainText(m_description);
    return true;
}

void PriceLevelDialog::on_simpanButton_clicked() {
    // 1. Ambil data dari UI
    QString name = ui->nameLineEdit->text().trimmed();
    QString desc = ui->plainTextEdit->toPlainText().trimmed();
    
    // 2. Validasi Input Dasar
    QStringList errs;
    if (name.isEmpty()) errs << "- Nama Level harus diisi";
    if (desc.isEmpty()) errs << "- Deskripsi harus diisi";
    
    if (!errs.isEmpty()) {
        QMessageBox::warning(this, "Periksa input", 
            QString("Tidak dapat menyimpan:\n%1").arg(errs.join("\n")));
        return;
    }

    // 3. Validasi Keunikan Nama
    PriceLevelManager plm;
    auto existing = plm.findByName(name);
    
    if (existing.has_value()) {
        int existingId = existing->value("id").toInt();
        
        // Jika ID yang ditemukan berbeda dengan ID yang sedang diedit, 
        // berarti nama tersebut sudah dipakai oleh data lain.
        if (existingId != m_priceLevelId) {
            QMessageBox::warning(this, "Nama Sudah Ada", 
                QString("Nama level '%1' sudah digunakan. Silakan gunakan nama lain.").arg(name));
            return;
        }
    }

    // 4. Eksekusi Simpan
    if (isDirty()) {
        if (commit()) {
            accept();
        } else {
            QMessageBox::critical(this, "Kesalahan", "Gagal menyimpan data ke database.");
        }
    } else {
        accept();
    }
}

bool PriceLevelDialog::isDirty() const 
{
    // Jika ID -1 (Data Baru), otomatis dianggap kotor/dirty jika input tidak kosong
    if (m_priceLevelId < 1) {
        return !ui->nameLineEdit->text().trimmed().isEmpty() || 
               !ui->plainTextEdit->toPlainText().trimmed().isEmpty();
    }

    // Cek apakah data di UI berbeda dengan data asli dari database
    return (m_name != ui->nameLineEdit->text().trimmed()) || 
           (m_description != ui->plainTextEdit->toPlainText().trimmed());
}

bool PriceLevelDialog::commit() 
{
    PriceLevelManager plm;
    QVariantMap data;
    data["level_name"] = ui->nameLineEdit->text().trimmed();
    data["description"] = ui->plainTextEdit->toPlainText().trimmed();

    if (m_priceLevelId < 1) {
        // Logika Create
        auto pl = plm.create(data);
        if (!pl.has_value()) return false;
        
        m_priceLevelId = pl->value("id").toInt();
        return true;
    } else {
        // Logika Update
        return plm.update(m_priceLevelId, data);
    }
}

void PriceLevelDialog::reject() 
{
    if (isDirty()) {
        auto resBtn = QMessageBox::question(this, "Konfirmasi",
            "Perubahan belum disimpan. Simpan sekarang?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (resBtn == QMessageBox::Save) {
            if (commit()) accept();
        } else if (resBtn == QMessageBox::Discard) {
            QDialog::reject();
        }
        // Jika Cancel, dialog tetap terbuka
    } else {
        QDialog::reject();
    }
}