#pragma once

#include "fieldmap.h"

#include <QDialog>
#include <QList>
#include <QSqlRecord>
#include <QVariantMap>

/**
 * FormMode
 * Determines whether the dialog is used for creating or modifying a record.
 */
enum class FormMode {
    Create,
    Modify
};

/**
 * FormDialog
 *
 * Generic base class for data entry dialogs backed by a QSqlRecord.
 *
 * Subclasses MUST implement:
 *   - setupFields()  — register FieldMap list and setup any custom widgets
 *   - onSave()       — handle INSERT (Create) or UPDATE (Modify) DB operation
 *
 * Subclasses CAN override:
 *   - onPrepareCreate()  — called after clearing fields in Create mode
 *   - onPrepareModify()  — called after populating fields in Modify mode
 *
 * Usage:
 *   dialog->prepareCreate();       // for new record
 *   dialog->prepareModify(record); // for editing existing record
 *   dialog->exec();
 */
class FormDialog : public QDialog {
    Q_OBJECT

public:
    explicit FormDialog(QWidget* parent = nullptr)
        : QDialog(parent), m_mode(FormMode::Create)
    {
        // connect accepted signal to internal handler
        connect(this, &QDialog::accepted, this, &FormDialog::handleAccepted);
    }

    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------

    void prepareCreate() {
        m_mode = FormMode::Create;
        m_originalRecord = QSqlRecord();
        setupFields();
        clearFields();
        onPrepareCreate();
    }

    void prepareModify(const QSqlRecord& record) {
        m_mode = FormMode::Modify;
        m_originalRecord = record;
        setupFields();
        populateFields();
        onPrepareModify();
    }

    FormMode mode()       const { return m_mode; }
    bool isCreateMode()   const { return m_mode == FormMode::Create; }
    bool isModifyMode()   const { return m_mode == FormMode::Modify; }

    bool isModified() const {
        if (isCreateMode()) return true;
        return !collect().isEmpty();
    }
    
    void accept() override {
        QVariantMap data = collect();

        if (data.isEmpty() && isModifyMode()) {
            QDialog::accept();  // tidak ada perubahan, tutup saja
            return;
        }

        if (onSave(data)) {
            QDialog::accept();  // sukses, baru tutup dialog
        }
        // gagal — dialog tetap terbuka, user bisa perbaiki
    }

protected:
    // -----------------------------------------------------------------------
    // Pure virtuals — subclass MUST implement
    // -----------------------------------------------------------------------

    virtual void setupFields() = 0;
    virtual bool onSave(const QVariantMap& data) = 0;

    // -----------------------------------------------------------------------
    // Optional hooks — subclass CAN override
    // -----------------------------------------------------------------------

    virtual void onPrepareCreate() {}
    virtual void onPrepareModify() {}

    // -----------------------------------------------------------------------
    // Helpers for subclass
    // -----------------------------------------------------------------------

    void setFields(const QList<FieldMap>& fields) {
        m_fields = fields;
    }

    const QSqlRecord& originalRecord() const {
        return m_originalRecord;
    }

    /**
     * collect()
     * - Create mode: returns ALL fields
     * - Modify mode: returns only MODIFIED fields (compared to original record)
     * Subclass can call this in onSave() and append extra fields (e.g. ComboBox).
     */
    QVariantMap collect() const {
        QVariantMap result;
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

                if (isCreateMode())
                    result[f.key] = current;
                else if (current != m_originalRecord.value(f.key))
                    result[f.key] = current;

            }, f.editor);
        }
        return result;
    }

private:
    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    void populateFields() {
        for (const auto& f : m_fields) {
            std::visit([&](auto* editor) {
                using T = std::decay_t<decltype(*editor)>;
                if constexpr (std::is_same_v<T, QLineEdit>)
                    editor->setText(m_originalRecord.value(f.key).toString());
                else if constexpr (std::is_same_v<T, QSpinBox>)
                    editor->setValue(m_originalRecord.value(f.key).toInt());
                else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                    editor->setPlainText(m_originalRecord.value(f.key).toString());
            }, f.editor);
        }
    }

    void clearFields() {
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

private slots:
    void handleAccepted() {
        QVariantMap data = collect();
        if (data.isEmpty() && isModifyMode()) {
            // nothing changed, no need to hit the DB
            return;
        }
        onSave(data);
    }

private:
    QList<FieldMap> m_fields;
    FormMode        m_mode;

protected:
    QSqlRecord      m_originalRecord;
};

// #include moc_formdialog.cpp
