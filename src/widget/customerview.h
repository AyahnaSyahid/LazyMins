#ifndef CUSTOMERTABLE_H
#define CUSTOMERTABLE_H

#include <QTableView>
#include <QSqlQueryModel>
#include <QStyledItemDelegate>

class CustomerContactModel;
class CustomerView : public QTableView
{
  Q_OBJECT
  
  public:
    explicit CustomerView(QWidget *parent=nullptr);
    ~CustomerView();
  
  private:
    CustomerContactModel *c_model
};


class CustomerContactModel : public QSqlQueryModel
{
  public:
    explicit CustomerContactModel(QObject *parent=nullptr) : QSqlQueryModel(parent) {}
    ~CustomerContactModel() {}
    Qt::ItemFlags flags(const QModelIndex& mi) const override;
};

class CustomerEditorDelegate : public QStyledItemDelegate
{
  public:
    CustomerEditorDelegate(QObject *parent=nullptr) : QStyledItemDelegate(parent);
    ~CustomerEditorDelegate() {}
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex& ix) const override;
}

#endif