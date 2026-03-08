
#pragma once

#include <QStyledItemDelegate>
#include <QComboBox>

class BooleanDelegate : public QStyledItemDelegate
{
  public:
    BooleanDelegate(QObject *parent) : QStyledItemDelegate(parent) {
      values = {{0, "True"}, {1, "False"}};
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
      model->setData(mi, cb->currentData(), Qt::EditRole) && model->setData(mi, cb->currentText(), Qt::DisplayRole);
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