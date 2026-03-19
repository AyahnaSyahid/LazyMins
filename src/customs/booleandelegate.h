
#pragma once

#include <QStyledItemDelegate>
#include <QComboBox>

class BooleanDelegate : public QStyledItemDelegate
{
  public:
    BooleanDelegate(QObject *parent) : QStyledItemDelegate(parent) {
      values = {{0, "False"}, {1, "True"}};
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option, const QModelIndex &mi) const override {
      auto cb = new QComboBox(parent);
      for(auto const &[key, val] : values.asKeyValueRange()) 
        cb->addItem(val, key);
      return cb;
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &mi) const override {
      auto cb = qobject_cast<QComboBox *>(editor);
      if(!cb) return ;
      model->setData(mi, cb->currentData(), Qt::EditRole);
    }

    QString displayText(const QVariant& p, const QLocale& loc) const override {
      int ip = p.toInt();
      return values.value(ip, "Unknown");
    }

    void setEditorData(QWidget *editor, const QModelIndex &mi) const override {
      auto cb = qobject_cast<QComboBox*>(editor);
      if(!cb) return;
      return cb->setCurrentIndex(mi.data(Qt::EditRole).toInt());
    }
    
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
      editor->setGeometry(option.rect);
    }
    
  private:
    QMap<int, QString> values;
};