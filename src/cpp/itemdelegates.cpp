#include "itemdelegates.h"
#include <QSpinBox>
#include <QLineEdit>
#include <QDateTime>
#include <QtDebug>

QWidget* DelegateHarga::createEditor(QWidget* parent, const QStyleOptionViewItem& opt, const QModelIndex& mi) const
{
    QSpinBox* sp = new QSpinBox(parent);
    sp->setRange(0, 2147483647);
    sp->setSingleStep(1000);
    sp->setButtonSymbols(sp->NoButtons);
    sp->setGroupSeparatorShown(true);
    return sp;
}

void DelegateHarga::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto sp = qobject_cast<QSpinBox*>(editor);
    sp->setAlignment(Qt::AlignRight);
    sp->setValue(index.data(Qt::EditRole).toDouble());
}

void DelegateHarga::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto sp = qobject_cast<QSpinBox*>(editor);
    model->setData(index, sp->value());
}

void DelegateHarga::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto sp = qobject_cast<QSpinBox*>(editor);
    sp->setMaximumSize(option.rect.size());
    sp->setGeometry(option.rect.adjusted(0, option.rect.height(), 0, option.rect.height()));
}

QWidget* DelegateQty::createEditor(QWidget* parent, const QStyleOptionViewItem& opt, const QModelIndex& mi) const
{
    auto e = qobject_cast<QSpinBox*>(DelegateHarga::createEditor(parent, opt, mi));
    e->setSingleStep(0.5);
    return e;
}

LihatHari::LihatHari(QObject* parent) : incTime(false), QStyledItemDelegate(parent) {}

void LihatHari::setIncludeTime(bool b)
{
    incTime = b;
}

QString LihatHari::displayText(const QVariant& v, const QLocale& loc) const
{
    if(incTime)
        return loc.toString(v.toDateTime(), "dddd, dd MMMM yyyy HH:mm:ss");
    else
        return loc.toString(v.toDateTime(), "dddd, dd MMMM yyyy");
}
