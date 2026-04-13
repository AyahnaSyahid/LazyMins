#include "formdialog.h"
#include <QMessageBox>

// FIX Bug 1: helper ini sekarang benar-benar digunakan di collect()
namespace {
bool equalsIgnoringType(const QVariant &a, const QVariant &b)
{
    if (a.isNull() && b.isNull()) return true;

    // Bandingkan sebagai string bila salah satu null (DB sering kirim null
    // untuk field teks kosong, sedangkan UI kirim empty string)
    if (a.isNull() || b.isNull()) return false;

    // Perbandingan numerik fuzzy untuk float/double
    const bool aIsReal = (a.typeId() == QMetaType::Double || a.typeId() == QMetaType::Float);
    const bool bIsReal = (b.typeId() == QMetaType::Double || b.typeId() == QMetaType::Float);
    if (aIsReal || bIsReal)
        return qFuzzyCompare(a.toDouble(), b.toDouble());

    // Perbandingan numerik lintas-tipe integer (int vs qlonglong, dll.)
    const bool aIsInt = (a.typeId() == QMetaType::Int || a.typeId() == QMetaType::LongLong
                         || a.typeId() == QMetaType::UInt || a.typeId() == QMetaType::ULongLong);
    const bool bIsInt = (b.typeId() == QMetaType::Int || b.typeId() == QMetaType::LongLong
                         || b.typeId() == QMetaType::UInt || b.typeId() == QMetaType::ULongLong);
    if (aIsInt && bIsInt)
        return a.toLongLong() == b.toLongLong();

    // Fallback: bandingkan sebagai string (tangani int vs string dari DB)
    if (a.userType() != b.userType())
        return a.toString() == b.toString();

    return a == b;
}
} // namespace

FormDialog::FormDialog(QWidget *parent)
    : QDialog(parent)
{}

void FormDialog::prepareCreate()
{
    m_mode = FormMode::Create;
    m_originalRecord = QSqlRecord();

    // FIX Bug 5: bersihkan list agar tidak menumpuk bila dialog di-reuse
    m_fields.clear();
    m_boundFields.clear();

    setupFields();
    setupBoundFields();
    clearFields();
    onPrepareCreate();
}

void FormDialog::prepareModify(const QSqlRecord &record)
{
    m_mode = FormMode::Modify;
    m_originalRecord = record;

    // FIX Bug 5: sama seperti prepareCreate
    m_fields.clear();
    m_boundFields.clear();

    setupFields();
    setupBoundFields();
    populateFields();
    onPrepareModify();
}

bool FormDialog::isModified() const
{
    // FIX Bug 7: Create mode — cek apakah ada field yang sudah diisi,
    // bukan langsung return true. Hindari juga double-call isInputAcceptable().
    if (isCreateMode()) {
        for (const auto &f : m_fields) {
            bool filled = false;
            std::visit([&](auto *editor) {
                using T = std::decay_t<decltype(*editor)>;
                if constexpr (std::is_same_v<T, QLineEdit>)
                    filled = !editor->text().isEmpty();
                else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                    filled = !editor->toPlainText().isEmpty();
                else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
                    filled = !qFuzzyIsNull(editor->value());
                else if constexpr (std::is_same_v<T, QSpinBox>)
                    filled = editor->value() != 0;
            }, f.editor);
            if (filled) return true;
        }
        return false;
    }

    // FIX Bug 4: Modify mode — gunakan versi collect() yang tidak memicu
    // isInputAcceptable() agar tidak ada double-warning saat accept().
    // Kita cukup periksa apakah ada perbedaan nilai, tanpa validasi penuh.
    for (const auto &f : m_fields) {
        bool changed = false;
        std::visit([&](auto *editor) {
            using T = std::decay_t<decltype(*editor)>;
            QVariant current;
            if constexpr (std::is_same_v<T, QLineEdit>)
                current = editor->text();
            else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
                current = editor->value();
            else if constexpr (std::is_same_v<T, QSpinBox>)
                current = editor->value();
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                current = editor->toPlainText();
            if (!equalsIgnoringType(current, m_originalRecord.value(f.key)))
                changed = true;
        }, f.editor);
        if (changed) return true;
    }
    for (const auto &[column, getter, setter, defVal] : m_boundFields) {
        if (!equalsIgnoringType(getter(), m_originalRecord.value(column)))
            return true;
    }
    return false;
}

void FormDialog::accept()
{
    // FIX Bug 4: panggil isInputAcceptable() sekali saja di sini,
    // lalu teruskan ke collect() tanpa validasi ulang.
    if (!isInputAcceptable()) return;  // dialog tetap terbuka

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
            QMessageBox::Ok);
        return;
    }
    QDialog::accept();
}

void FormDialog::setFields(const QList<FieldMap> &fields)
{
    m_fields = fields;
}

void FormDialog::addBoundField(
    const QString &column,
    ValueGetter getter,
    ValueSetter setter,
    const QVariant &defaultValue)  // FIX Bug 5 & 8
{
    m_boundFields.push_back({column, std::move(getter), std::move(setter), defaultValue});
}

QVariantMap FormDialog::collect() const
{
    // FIX Bug 4: isInputAcceptable() sudah dipanggil di accept() sebelum collect().
    // collect() tidak memanggil lagi agar tidak ada double-dialog.
    // Catatan: isModified() juga menyalin logika ini secara terpisah supaya aman.

    QVariantMap result;

    // 1. Field standar
    for (const auto &f : m_fields) {
        std::visit([&](auto *editor) {
            using T = std::decay_t<decltype(*editor)>;
            QVariant current;

            if constexpr (std::is_same_v<T, QLineEdit>)
                current = editor->text();
            else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
                current = editor->value();
            else if constexpr (std::is_same_v<T, QSpinBox>)
                current = editor->value();
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                current = editor->toPlainText();

            if (isCreateMode()) {
                result[f.key] = current;
            }
            // FIX Bug 2: gunakan equalsIgnoringType() bukan !=
            else if (!equalsIgnoringType(current, m_originalRecord.value(f.key))) {
                result[f.key] = current;
            }
        }, f.editor);
    }

    // 2. Bound fields
    for (const auto &[column, getter, setter, defVal] : m_boundFields) {
        QVariant current = getter();
        if (isCreateMode()) {
            result[column] = current;
        }
        // FIX Bug 2: sama, gunakan equalsIgnoringType()
        else if (!equalsIgnoringType(current, m_originalRecord.value(column))) {
            result[column] = current;
        }
    }
    return result;
}

void FormDialog::populateFields()
{
    for (const auto &f : m_fields) {
        std::visit([&](auto *editor) {
            using T = std::decay_t<decltype(*editor)>;
            QVariant val = m_originalRecord.value(f.key);

            if constexpr (std::is_same_v<T, QLineEdit>)
                editor->setText(val.toString());
            // FIX Bug 3: gunakan toDouble() bukan toInt() untuk QDoubleSpinBox
            else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
                editor->setValue(val.toDouble());
            else if constexpr (std::is_same_v<T, QSpinBox>)
                editor->setValue(val.toInt());
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                editor->setPlainText(val.toString());
        }, f.editor);
    }

    for (const auto &bf : m_boundFields) {
        if (bf.setter) {
            bf.setter(m_originalRecord.value(bf.column));
        }
    }
}

void FormDialog::clearFields()
{
    for (const auto &f : m_fields) {
        std::visit([](auto *editor) {
            using T = std::decay_t<decltype(*editor)>;
            if constexpr (std::is_same_v<T, QLineEdit>)
                editor->clear();
            else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
                editor->setValue(0.0);
            else if constexpr (std::is_same_v<T, QSpinBox>)
                editor->setValue(0);
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                editor->clear();
        }, f.editor);
    }

    // FIX Bug 8: reset bound fields ke defaultValue bila setter tersedia
    for (const auto &bf : m_boundFields) {
        if (bf.setter && bf.defaultValue.isValid()) {
            bf.setter(bf.defaultValue);
        }
    }
}
