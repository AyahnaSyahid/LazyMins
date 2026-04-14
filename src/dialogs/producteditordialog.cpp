#include "producteditordialog.h"
#include "ui_producteditordialog.h"
#include "src/managers/managers.h"
#include <QMessageBox>

ProductEditorDialog::ProductEditorDialog(QWidget *p) :
    QDialog(p),
    ui(new Ui::ProductEditorDialog), 
    m_productId(-1)
{
    ui->setupUi(this);
}

ProductEditorDialog::~ProductEditorDialog() { delete ui; }

bool ProductEditorDialog::setProductId(int p)
{
    ProductManager pm;
    auto opt_p = pm.getById(p);
    if(!opt_p.has_value()) return false;

    m_productId = p;
    QSqlRecord rec = *opt_p;

    // Simpan ke variabel internal
    m_origName   = rec.value("name").toString();
    m_origSku    = rec.value("sku").toString();
    m_origCatId  = rec.value("category_id").toInt();
    m_origActive = rec.value("is_active").toInt();
    m_origUnit   = rec.value("unit").toString();
    m_origDesc   = rec.value("description").toString();

    // Set ke UI
    ui->namaLineEdit->setText(m_origName);
    ui->sKULineEdit->setText(m_origSku);
    ui->productCategoriesComboBox->setCurrentId(m_origCatId);
    ui->aktifComboBox->setCurrentIndex(m_origActive == 1 ? 0 : 1); // Asumsi Index 0 = Ya, 1 = Tidak
    ui->comboUnit->setCurrentText(m_origUnit);
    ui->descPlainTextEdit->setPlainText(m_origDesc);

    return true;
}

bool ProductEditorDialog::isDirty() const {
    if (m_productId < 1) {
        return !ui->namaLineEdit->text().trimmed().isEmpty() || 
               !ui->sKULineEdit->text().trimmed().isEmpty();
    }

    return (m_origName   != ui->namaLineEdit->text().trimmed()) ||
           (m_origSku    != ui->sKULineEdit->text().trimmed()) ||
           (m_origCatId  != ui->productCategoriesComboBox->currentId()) ||
           (m_origActive != (ui->aktifComboBox->currentText() == "Ya" ? 1 : 0)) ||
           (m_origUnit   != ui->comboUnit->currentText()) ||
           (m_origDesc   != ui->descPlainTextEdit->toPlainText().trimmed());
}

void ProductEditorDialog::on_simpanButton_clicked() {
    QString name = ui->namaLineEdit->text().trimmed();
    QString sku  = ui->sKULineEdit->text().trimmed();

    // 1. Validasi Input Kosong
    QStringList errs;
    if(name.isEmpty()) errs << "- Nama Produk harus diisi";
    if(sku.isEmpty())  errs << "- SKU harus diisi";
    
    if (!errs.isEmpty()) {
        QMessageBox::warning(this, "Periksa input", errs.join("\n"));
        return;
    }

    // 2. Validasi Unik (Nama & SKU)
    ProductManager pm;
    // Cek Nama
    auto existName = pm.getByName(name);
    if(existName.has_value() && existName->value("id").toInt() != m_productId) {
        QMessageBox::warning(this, "Kesalahan", "Nama produk sudah digunakan.");
        return;
    }
    // Cek SKU
    auto existSku = pm.getBySku(sku); // Asumsi ada fungsi findBySku
    if(existSku.has_value() && existSku->value("id").toInt() != m_productId) {
        QMessageBox::warning(this, "Kesalahan", "SKU sudah digunakan.");
        return;
    }

    // 3. Proses Simpan
    if (isDirty()) {
        if (commit()) accept();
        else QMessageBox::critical(this, "Kesalahan", "Gagal menyimpan ke database.");
    } else {
        accept();
    }
}

bool ProductEditorDialog::commit() {
    ProductManager pm;
    QVariantMap data;
    data["name"]        = ui->namaLineEdit->text().trimmed();
    data["sku"]         = ui->sKULineEdit->text().trimmed();
    data["category_id"] = ui->productCategoriesComboBox->currentId();
    data["is_active"]   = (ui->aktifComboBox->currentText() == "Ya" ? 1 : 0);
    data["unit"]        = ui->comboUnit->currentText();
    data["description"] = ui->descPlainTextEdit->toPlainText().trimmed();

    if (m_productId < 1) {
        auto res = pm.create(data);
        if(!res.has_value()) return false;
        m_productId = res->value("id").toInt();
        return true;
    } else {
        return pm.update(m_productId, data);
    }
}

void ProductEditorDialog::reject() {
    if (isDirty()) {
        auto resBtn = QMessageBox::question(this, "Konfirmasi",
            "Perubahan belum disimpan. Simpan sekarang?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (resBtn == QMessageBox::Save) {
            if (commit()) accept();
        } else if (resBtn == QMessageBox::Discard) {
            QDialog::reject();
        }
    } else {
        QDialog::reject();
    }
}