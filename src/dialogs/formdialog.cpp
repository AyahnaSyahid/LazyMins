#include "formdialog.h"
#include <QMessageBox>

FormDialog::FormDialog(QWidget* parent)
    : QDialog(parent)
{
    // Tidak lagi connect accepted secara otomatis – kita handle manual di accept()
}

void FormDialog::prepareCreate()
{
    m_mode = FormMode::Create;
    m_originalRecord = QSqlRecord();
    setupFields();
    setupBoundFields();
    clearFields();
    onPrepareCreate();
}

void FormDialog::prepareModify(const QSqlRecord& record)
{
    m_mode = FormMode::Modify;
    m_originalRecord = record;
    setupFields();
    setupBoundFields();
    populateFields();
    onPrepareModify();
}

bool FormDialog::isModified() const
{
    if (isCreateMode()) {
        return true;
    }

    return !collect().isEmpty();
}

void FormDialog::accept()
{
    QVariantMap changes = collect();
    
    if (isModifyMode() && changes.isEmpty()) {
        QDialog::accept();
        return;
    }


    if (!onSave(changes)) {
        QMessageBox::warning(
            this,
            "Gagal Menyimpan",
            "Data tidak dapat disimpan.\n"
            "Mohon periksa kembali isian Anda.",
            QMessageBox::Ok
        );
        // jika gagal → dialog tetap terbuka, user bisa koreksi
        return;
    }
    QDialog::accept();
}

void FormDialog::setFields(const QList<FieldMap>& fields)
{
    m_fields = fields;
}

void FormDialog::addBoundField( const QString& column, ValueGetter getter, ValueSetter setter)
{
    m_boundFields.push_back({column, std::move(getter), std::move(setter)});
}

QVariantMap FormDialog::collect() const
{
    QVariantMap result;

    // 1. Field standar (yang terdaftar via FieldMap)
    for (const auto& f : m_fields) {
        std::visit([&](auto* editor) {
            using T = std::decay_t<decltype(*editor)>;
            QVariant current;

            if constexpr (std::is_same_v<T, QLineEdit>)
                current = editor->text();
            else if constexpr (std::is_same_v<T, QSpinBox>)
                current = editor->value();
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                current = editor->toPlainText();
            // tambahkan tipe lain jika diperlukan

            if (isCreateMode()) {
                result[f.key] = current;
            }
            else if (current != m_originalRecord.value(f.key)) {
                result[f.key] = current;
            }
        }, f.editor);
    }

    // 2. Field tambahan (bound fields)
    for (const auto& [column, getter, setter] : m_boundFields) {
        QVariant current = getter();

        if (isCreateMode()) {
            result[column] = current;
        }
        else if (current != m_originalRecord.value(column)) {
            result[column] = current;
        }
    }
    qDebug() << result;
    return result;
}

void FormDialog::populateFields()
{
    // populate field standar (seperti sebelumnya)
    for (const auto& f : m_fields) {
        std::visit([&](auto* editor) {
            using T = std::decay_t<decltype(*editor)>;
            QVariant val = m_originalRecord.value(f.key);

            if constexpr (std::is_same_v<T, QLineEdit>)
                editor->setText(val.toString());
            else if constexpr (std::is_same_v<T, QSpinBox>)
                editor->setValue(val.toInt());
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                editor->setPlainText(val.toString());
            // tambahkan tipe lain jika perlu
        }, f.editor);
    }

    // populate bound fields (custom widgets)
    for (const auto& bf : m_boundFields) {
        if (bf.setter) {
            QVariant value = m_originalRecord.value(bf.column);
            bf.setter(value);
        }
    }
}

void FormDialog::clearFields()
{
    for (const auto& f : m_fields) {
        std::visit([](auto* editor) {
            using T = std::decay_t<decltype(*editor)>;
            if constexpr (std::is_same_v<T, QLineEdit>)
                editor->clear();
            else if constexpr (std::is_same_v<T, QSpinBox>)
                editor->setValue(0);
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                editor->clear();
        }, f.editor);
    }
}