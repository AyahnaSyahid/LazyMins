#pragma once
#include <QStyledItemDelegate>
#include <functional>

class FlexibleDelegate : public QStyledItemDelegate
{
public:
    using Displayer = std::function<QString(const QVariant &, const QLocale &)>;
    using Styler = std::function<void(QStyleOptionViewItem &, const QModelIndex &)>;
    using Creator = std::function<QWidget *(QWidget *, const QStyleOptionViewItem &, const QModelIndex &)>;

    struct Option {
        Displayer displayer;
        Styler styler;
        Creator creator;
    };
    
    // factory method untuk membuat delegate dengan opsi tertentu
    static FlexibleDelegate* create(const Option& option, QObject *parent = nullptr)
    {
        auto delegate = new FlexibleDelegate(parent);
        delegate->m_displayer = option.displayer;
        delegate->m_styler = option.styler;
        return delegate;
    };

    explicit FlexibleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void clearDisplayer()
    {
        m_displayer = nullptr;
    }

    QString displayText(const QVariant &value, const QLocale &locale) const override
    {
        if (m_displayer)
            return m_displayer(value, locale);

        return QStyledItemDelegate::displayText(value, locale);
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (m_creator)
            return m_creator(parent, option, index);

        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    FlexibleDelegate &setCreator(const Creator &creator)
    {
        m_creator = creator;
        return *this;
    }
    
    FlexibleDelegate &setDisplayer(const Displayer &displayer)
    {
        m_displayer = displayer;
        return *this;
    }

    FlexibleDelegate &setStyler(const Styler &styler)
    {
        m_styler = styler;
        return *this;
    }

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (m_styler)
            m_styler(*option, index);
    }

private:
    Displayer m_displayer;
    Styler m_styler;
    Creator m_creator;
};