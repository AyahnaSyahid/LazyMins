#include "notacolumndelegate.h"
#include <QSpinBox>

QString NotaNoDelegate::displayText(const QVariant& value, const QLocale& locale) const {
  Q_UNUSED(locale);
  return QString("%1").arg(value.toString(), 4, QChar('0'));
}

QString NotaHPDelegate::displayText(const QVariant& value, const QLocale& locale) const {
  return QLocale().toString(value.toInt());
}

QWidget *NotaHPDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex& mi) const {
  auto sp = new QSpinBox(parent);
  sp->setButtonSymbols(QSpinBox::NoButtons);
  sp->setRange(0,1'000'000);
  sp->setGroupSeparatorShown(true);
  sp->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  if(mi.column() == 2) {
    sp->setProperty("indexColumn", 2);
    sp->setSingleStep(1000);
    sp->setAccelerated(true);
  } else {
    sp->setProperty("indexColumn", 3);
  }
  return sp;
}

void NotaHPDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
  auto sp = qobject_cast<QSpinBox*>(editor);
  int x = 0;
  if (sp->property("indexColumn").toInt() == 2) {
    x = index.siblingAtColumn(3).data(Qt::EditRole).toInt();
  } else {
    x = index.siblingAtColumn(2).data(Qt::EditRole).toInt();
  }
  model->setData(index, sp->value(), Qt::EditRole);
  model->setData(index.siblingAtColumn(4), sp->value() * x, Qt::EditRole);
}

void NotaHPDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
  editor->setGeometry(option.rect);
}

QString NotaSubTotalDelegate::displayText(const QVariant& val, const QLocale& locale) const {
  return QLocale().toString(val.toInt());
}