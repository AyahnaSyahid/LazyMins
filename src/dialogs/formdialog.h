#pragma once

#include "fieldmap.h"

#include <QDialog>
#include <QList>
#include <QSqlRecord>
#include <QVariantMap>
#include <functional>

enum class FormMode {
    Create,
    Modify
};

class FormDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FormDialog(QWidget* parent = nullptr);
    
    using ValueGetter = std::function<QVariant()>;
    using ValueSetter = std::function<void(const QVariant&)>;

    struct BoundField {
        QString column;
        ValueGetter getter;
        ValueSetter setter;
    };
    
    void prepareCreate();
    void prepareModify(const QSqlRecord& record);

    FormMode mode() const             { return m_mode; }
    bool isCreateMode() const         { return m_mode == FormMode::Create; }
    bool isModifyMode() const         { return m_mode == FormMode::Modify; }
    bool isModified() const;

    void accept() override;

protected:
    virtual void setupFields() = 0;
    virtual void setupBoundFields() {}   // opsional – override jika ada field custom

    virtual bool onSave(const QVariantMap& changes) = 0;

    virtual void onPrepareCreate() {}
    virtual void onPrepareModify() {}

    // Helpers untuk subclass
    void setFields(const QList<FieldMap>& fields);
    void addBoundField(
        const QString& column,
        ValueGetter getter,
        ValueSetter setter = nullptr
    );
    
    const QSqlRecord& originalRecord() const { return m_originalRecord; }

    // Fungsi utama: mengumpulkan semua perubahan (atau semua field di mode create)
    QVariantMap collect() const;

private:
    void clearFields();
    void populateFields();

private:
    QList<FieldMap> m_fields;
    QList<BoundField> m_boundFields;

    FormMode    m_mode = FormMode::Create;
    QSqlRecord  m_originalRecord;
};