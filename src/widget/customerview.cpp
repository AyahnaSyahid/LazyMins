#include "customerview.h"
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QStyledItemDelegate>

namespace {
  class Delegate : public QStyledItemDelegate
  {
    public:
      Delegate(QObject *parent=nullptr) : QStyledItemDelegate(parent) {}
      ~Delegate() {}
    protected:
      void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex& ix) const override {
        QStyledItemDelegate::initStyleOption(opt, ix);
          if(ix.column() == 1) opt->displayAlignment = Qt::AlignCenter;
      }
  };
}

CustomerView::CustomerView(QWidget *p) 
  : c_model(new CustomerContactModel(this)),
    QTableView(p)
{
  c_model->setQuery(R"--(
    SELECT customer AS Nama, 
           customer_phone AS CP 
      FROM invoices 
  GROUP BY customer, 
           customer_phone 
  ORDER BY customer;
    )--", QSqlDatabase::database("JUST-INV_DB", true));
  
  auto sortModel = new QSortFilterProxyModel(this);
  sortModel->setSourceModel(c_model);
  setModel(sortModel);
  horizontalHeader()->setStretchLastSection(true);
  verticalHeader()->hide();
  adjustSize();
  auto pal = palette();
  pal.setColor(QPalette::AlternateBase, QColor(228, 227, 235));
  setPalette(pal);
  setAlternatingRowColors(true);
  setItemDelegate(new Delegate(this));
}

CustomerView::~CustomerView() {}

Qt::ItemFlags CustomerContactModel::flags(const QModelIndex &mi) const
{
  return Qt::ItemFlags (Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

bool CustomerContactModel::setData(const QModelIndex& mi, const QVariant &val, int role)
{
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
    setQuery(query().lastQuery(), db);
    emit dataChanged(mi, mi, QList<int> {Qt::EditRole, Qt::DisplayRole});
    return true;
  }
  return false;
}