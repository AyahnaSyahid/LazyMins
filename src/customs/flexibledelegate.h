#pragma once
#include <QStyledItemDelegate>
#include <functional>

class FlexibleDelegate : public QStyledItemDelegate
{
public:
    using Displayer   = std::function<QString(const QVariant &, const QLocale &)>;
    using Styler      = std::function<void(QStyleOptionViewItem &, const QModelIndex &)>;
    using Creator     = std::function<QWidget *(QWidget *, const QStyleOptionViewItem &, const QModelIndex &)>;
    using DataSetter  = std::function<void(QWidget *, const QModelIndex &)>;
    using ModelSetter = std::function<void(QWidget *, QAbstractItemModel *, const QModelIndex &)>;

    struct Option {
        Displayer   displayer;
        Styler      styler;
        Creator     creator;
        DataSetter  dataSetter;   // tambahan
        ModelSetter modelSetter;  // tambahan
    };

    // Factory method
    static FlexibleDelegate* create(const Option& option, QObject* parent = nullptr)
    {
        auto* d = new FlexibleDelegate(parent);
        d->m_displayer   = option.displayer;
        d->m_styler      = option.styler;
        d->m_creator     = option.creator;     // ✅ fix: tambahkan ini
        d->m_dataSetter  = option.dataSetter;  // ✅ fix: tambahkan ini
        d->m_modelSetter = option.modelSetter; // ✅ fix: tambahkan ini
        return d;
    }

    explicit FlexibleDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent) {}

    // --- Overrides ---

    QString displayText(const QVariant& value, const QLocale& locale) const override
    {
        if (m_displayer)
            return m_displayer(value, locale);
        return QStyledItemDelegate::displayText(value, locale);
    }

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override
    {
        if (m_creator)
            return m_creator(parent, option, index);
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    // ✅ fix: tanpa ini, custom creator tidak bisa terisi datanya
    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        if (m_dataSetter)
            return m_dataSetter(editor, index);
        QStyledItemDelegate::setEditorData(editor, index);
    }

    // ✅ fix: tanpa ini, hasil edit tidak tersimpan ke model
    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override
    {
        if (m_modelSetter)
            return m_modelSetter(editor, model, index);
        QStyledItemDelegate::setModelData(editor, model, index);
    }

    // --- Fluent setters ---

    FlexibleDelegate& setCreator(const Creator& creator)
    {
        m_creator = creator;
        return *this;
    }

    FlexibleDelegate& setDisplayer(const Displayer& displayer)
    {
        m_displayer = displayer;
        return *this;
    }

    FlexibleDelegate& setStyler(const Styler& styler)
    {
        m_styler = styler;
        return *this;
    }

    FlexibleDelegate& setDataSetter(const DataSetter& dataSetter)
    {
        m_dataSetter = dataSetter;
        return *this;
    }

    FlexibleDelegate& setModelSetter(const ModelSetter& modelSetter)
    {
        m_modelSetter = modelSetter;
        return *this;
    }

    void clearDisplayer() { m_displayer = nullptr; }

protected:
    void initStyleOption(QStyleOptionViewItem* option,
                         const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (m_styler)
            m_styler(*option, index);
    }

private:
    Displayer   m_displayer;
    Styler      m_styler;
    Creator     m_creator;
    DataSetter  m_dataSetter;   // ✅ tambahan
    ModelSetter m_modelSetter;  // ✅ tambahan
};