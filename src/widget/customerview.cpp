#include "customerview.h"
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>

CustomerView::CustomerView(QWidget *p) 
  : c_model(new CustomerContactModel(this)),
    QTableView(p)
{
  c_model->setQuery("SELECT customer AS Nama, customer_phone AS CP FROM invoices GROUP BY customer, customer_phone ORDER BY customer;", QSqlDatabase::database("JUST-INV_DB", true));
  auto sortModel = new QSortFilterProxyModel(this);
  sortModel->setSourceModel(c_model);
  setModel(sortModel);
  horizontalHeader()->setStretchLastSection(true);
  verticalHeader()->hide();
  adjustSize();
  auto pal = palette();
  pal.setColor(QPalette::AlternateBase, QColor(228,227, 235));
  setPalette(pal);
  setAlternatingRowColors(true);
}

CustomerView::~CustomerView() {}

Qt::ItemFlags CustomerContactModel::flags(const QModelIndex &mi) const
{
  return Qt::ItemFlags (Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

bool CustomerContactModel::setData(const QModelIndex& mi, const QVariant &val, int role)
{
  // qDebug() << "Setting Index :" << mi;
  if (role != Qt::EditRole) return false;
  if (!mi.isValid()) return false;
  
  QString newData = val.toString();
  if(newData.toLower() == mi.data().toString().toLower()) return false;
  if (newData.isEmpty()) return false;
  auto db = QSqlDatabase::database("JUST-INV_DB", true);
  QSqlQuery q(db);
  
  if (mi.column() == 0) {
    // Edit Name
    q.prepare("UPDATE invoices SET customer = :new_data WHERE customer = :old_data AND ( customer_phone = :customer_phone OR customer_phone IS NULL )");
    q.bindValue(":new_data", newData);
    q.bindValue(":old_data", mi.data(Qt::EditRole).toString());
    q.bindValue(":customer_phone", mi.siblingAtColumn(1).data(Qt::EditRole).toString());
  } else {
    // Edit Phone
    q.prepare("UPDATE invoices SET customer_phone = :new_data WHERE (customer_phone = :old_data OR customer_phone IS NULL) AND customer = :customer");
    q.bindValue(":new_data", newData);
    q.bindValue(":old_data", mi.data(Qt::EditRole));
    q.bindValue(":customer", mi.siblingAtColumn(0).data(Qt::EditRole).toString());
  }
  
  if (q.exec()) {
    // Refresh query
    // qDebug() << "exec OK";
    setQuery(query().lastQuery(), db);
    emit dataChanged(mi, mi, QList<int> {Qt::EditRole, Qt::DisplayRole});
    return true;
  }
  // qDebug() << "Query Exec Fail";
  return false;
}

/***
void CustomerEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex& mi) const {
  auto sortModel = qobject_cast<QSortFilterProxyModel*>(model);
  auto sourceModel = qobject_cast<QSqlQueryModel*>(sortModel->sourceModel());
  auto ed = qobject_cast<QLineEdit*>(editor);
  if(!ed) return ;
  if(!sortModel) return ;
  if(!sourceModel) return ;
  QString newData = ed->text();
  if(newData.isEmpty()) return;
  auto six = sortModel->mapToSource(mi);
  const auto &db = QSqlDatabase::database("JUST-INV_DB", true);
  QSqlQuery q(db);
  if (mi.column() == 0) {
    // Edit Name
    q.prepare("UPDATE invoices SET customer = :new_data WHERE customer = :old_data AND customer_phone = :customer_phone");
    q.bindValue(":new_data", newData);
    q.bindValue(":old_data", six.data(Qt::EditRole).toString());
    q.bindValue(":customer_phone", six.siblingAtColumn(1).data(Qt::EditRole).toString());
  } else {
    // Edit Phone
    q.prepare("UPDATE invoices SET customer_phone = :new_data WHERE customer_phone = :old_data AND customer = :customer");
    q.bindValue(":new_data", newData);
    q.bindValue(":old_data", six.data(Qt::EditRole).toString());
    q.bindValue(":customer", six.siblingAtColumn(0).data(Qt::EditRole).toString());
  }
  if (q.exec()) {
    auto sq = sourceModel->query().lastQuery();
    sourceModel->setQuery(sq, db);
  }
}
***/