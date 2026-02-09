#ifndef NOTACOLUMN1DELEGATE_H
#define NOTACOLUMN1DELEGATE_H

#include <QStyledItemDelegate>

class NotaDelegate : public QStyledItemDelegate
{
  public:
    NotaDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
  
  protected:
    void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex& ix) const override {
      QStyledItemDelegate::initStyleOption(opt, ix);
      switch (ix.column()) {
        case 0:
          opt->displayAlignment = Qt::AlignCenter;
          break;
        case 1:
          opt->displayAlignment = Qt::AlignLeft | Qt::AlignHCenter;
          break;
        default:
          opt->displayAlignment = Qt::AlignRight | Qt::AlignHCenter;
      }
    }
};

class NotaNoDelegate : public NotaDelegate
{
  public:
    NotaNoDelegate(QObject *parent=nullptr) : NotaDelegate(parent) {}
    QString displayText(const QVariant& value, const QLocale& locale) const override;
};

class NotaHPDelegate : public NotaDelegate
{
  public:
    NotaHPDelegate(QObject *parent=nullptr) : NotaDelegate(parent) {}
    QString displayText(const QVariant& value, const QLocale& locale) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

class NotaSubTotalDelegate : public NotaDelegate
{
  public:
    NotaSubTotalDelegate(QObject *parent=nullptr) : NotaDelegate(parent) {}
    QString displayText(const QVariant& value, const QLocale& locale) const override;
};

#endif