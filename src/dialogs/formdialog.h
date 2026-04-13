#pragma once

#include "fieldmap.h"

#include <QDialog>
#include <QList>
#include <QSqlRecord>
#include <QVariantMap>
#include <functional>

enum class FormMode
{
    Create,
    Modify
};

class FormDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FormDialog(QWidget *parent = nullptr);
    virtual ~FormDialog() = default;
    using ValueGetter = std::function<QVariant()>;
    using ValueSetter = std::function<void(const QVariant &)>;

    struct BoundField
    {
        QString column;
        ValueGetter getter;
        ValueSetter setter;
        QVariant defaultValue;
    };

    void prepareCreate();
    void prepareModify(const QSqlRecord &record);

    FormMode mode() const { return m_mode; }
    bool isCreateMode() const { return m_mode == FormMode::Create; }
    bool isModifyMode() const { return m_mode == FormMode::Modify; }
    // FIX Bug 7: isModified() tidak lagi memanggil collect() pada Create mode
    bool isModified() const;
    void accept() override;

    virtual bool isInputAcceptable() const { return true; }

protected:
    virtual void setupFields() = 0;
    virtual void setupBoundFields() {}

    virtual bool onSave(const QVariantMap &changes) = 0;

    virtual void onPrepareCreate() {}
    virtual void onPrepareModify() {}

    void setFields(const QList<FieldMap> &fields);

    void addBoundField(
        const QString &column,
        ValueGetter getter,
        ValueSetter setter = nullptr,
        const QVariant &defaultValue = QVariant());

    const QSqlRecord &originalRecord() const { return m_originalRecord; }

    virtual QVariantMap collect() const;

protected:
    QList<FieldMap> m_fields;
    void clearFields();

private:
    void populateFields();

    QList<BoundField> m_boundFields;

    FormMode m_mode = FormMode::Create;
    QSqlRecord m_originalRecord;
};
