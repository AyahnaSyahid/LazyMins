#include "customerview.h"
#include <QSortFilterProxyModel>
#include <QLineEdit>


CustomerView::CustomerView(QWidget *p) 
  : c_model(new CustomerContactModel(this)),
    QTableView(p)
{
  auto sortModel = new QSortFilterProxyModel(this);
  sortModel->setSourceModel(c_model);
  setModel(sortModel);
}

CustomerView::~CustomerView() {}

Qt::ItemFlags CustomerContactModel::flags(const QModelIndex &mi) const
{
  return Qt::ItemFlags (Qt::ItemIsEditable | Qt::ItemIsVisible | Qt::ItemIsEnabled);
}

void CustomerEditorDelegate(QWidget *editor, QAbstractItemModel *model, const QModelIndex& mi) const {
  auto ed = qobject_cast<QLineEdit*>(editor);
  if (ed) {
    
  }
}