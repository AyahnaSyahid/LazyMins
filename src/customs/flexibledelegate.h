#include <QStyledItemDelegate>
#include <functional>

class FlexibleDelegate : public QStyledItemDelegate
{
public:
    using Displayer = std::function<QString(const QVariant&, const QLocale&)>;

    explicit FlexibleDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    void setDisplayer(Displayer displayer)
    {
        m_displayer = std::move(displayer);
    }

    void clearDisplayer()
    {
        m_displayer = nullptr;
    }

    QString displayText(const QVariant& value, const QLocale& locale) const override
    {
        if (m_displayer)
            return m_displayer(value, locale);

        return QStyledItemDelegate::displayText(value, locale);
    }

private:
    Displayer m_displayer;
};