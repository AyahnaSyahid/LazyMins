#include "../databaseinterface.h"
#include "customerview.h"
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QLabel>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
    c_view(new QTableView(this)),
    filterEdit(new QLineEdit(this)),
    RealTimeDataWidget(p)
{
  c_model->setQuery(R"--(
    SELECT customer,
       customer_phone
  FROM invoices
 WHERE created_at = (
        SELECT MAX(created_at) 
          FROM invoices t2
         WHERE t2.customer = invoices.customer )
 GROUP BY customer;
    )--", DatabaseInterface::instance().database());
  
  auto sortModel = new QSortFilterProxyModel(this);
  sortModel->setSourceModel(c_model);
  sortModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  sortModel->setFilterKeyColumn(-1);
  c_view->setModel(sortModel);
  c_view->horizontalHeader()->setStretchLastSection(true);
  c_view->verticalHeader()->hide();
  c_view->verticalHeader()->setMinimumSectionSize(20);
  c_view->verticalHeader()->setDefaultSectionSize(22);
  auto pal = c_view->palette();
  pal.setColor(QPalette::AlternateBase, QColor(228, 227, 235));
  c_view->setPalette(pal);
  c_view->setAlternatingRowColors(true);
  c_view->setItemDelegate(new Delegate(this));
  auto lay = new QVBoxLayout();
  auto fl = new QHBoxLayout();
  fl->addWidget(new QLabel("Filter", this), 0);
  fl->addWidget(filterEdit, 0);
  fl->addStretch(1);
  lay->addLayout(fl, 0);
  lay->addWidget(c_view, 1);
  setLayout(lay);
  connect(filterEdit, &QLineEdit::textChanged, sortModel, &QSortFilterProxyModel::setFilterFixedString);
  connect(&DatabaseInterface::instance(), &DatabaseInterface::tableUpdate, this, &CustomerView::reloadModelData);
  adjustSize();
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
  QSqlQuery q(DatabaseInterface::instance().database());
  
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
    setQuery(query().lastQuery(), DatabaseInterface::instance().database());
    emit dataChanged(mi, mi, QList<int> {Qt::EditRole, Qt::DisplayRole});
    return true;
  }
  return false;
}

void CustomerView::reloadModelData(const QList<QString> &tables)
{
  if(tables.indexOf("invoices") == -1 || tables.count() < 1) return;
  QString query(c_model->query().lastQuery());
  c_model->setQuery(query, DatabaseInterface::instance().database());
}