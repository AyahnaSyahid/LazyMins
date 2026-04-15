#include "productpickerdialog.h"
#include "ui_productpickerdialog.h"

#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QHeaderView>

#include "src/managers/basemanager.h"

namespace {
  class Delegate : public QStyledItemDelegate
  {
    public:
      Delegate(QObject *p) : QStyledItemDelegate(p) {}
    
    protected:
      void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex& i) const override {
        QStyledItemDelegate::initStyleOption(opt, i);
        switch (i.column()) {
          case 0:
            opt->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            break;
          case 3:
            opt->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            opt->text             = QString("%L1").arg(i.data().toDouble(), 2);
            break;
        }
      }
  };
}

ProductPickerDialog::ProductPickerDialog(QWidget *p)
: ui(new Ui::ProductPickerDialog), QDialog(p)
{
  ui->setupUi(this);
  auto qm = new QSqlQueryModel(this);
  
  ui->productView->verticalHeader()->setMinimumSectionSize(18);
  ui->productView->verticalHeader()->setDefaultSectionSize(20);
  ui->productView->verticalHeader()->hide();
  ui->productView->setAlternatingRowColors(true);
  
  QString query(R"-(
SELECT id, name, description, stock
  FROM products
 WHERE is_active = 1;
  )-");
  
  qm->setQuery(query, BaseManager::connection);
  qm->setHeaderData(0, Qt::Horizontal, "ID", Qt::DisplayRole);
  qm->setHeaderData(1, Qt::Horizontal, "Nama", Qt::DisplayRole);
  qm->setHeaderData(2, Qt::Horizontal, "Deskripsi", Qt::DisplayRole);
  qm->setHeaderData(3, Qt::Horizontal, "Stok", Qt::DisplayRole);

  auto pr = new QSortFilterProxyModel(this);
  pr->setSourceModel(qm);
  pr->setFilterKeyColumn(-1);
  
  ui->productView->setModel(pr);
  
  ui->productView->hideColumn(0);
  ui->productView->resizeColumnsToContents();
  ui->productView->setItemDelegate(new Delegate(this));
  ui->productView->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->productView->setSelectionBehavior(QAbstractItemView::SelectRows);
  
  auto filterTimer = new QTimer(this);
  filterTimer->setInterval(300);
  connect(ui->lineEdit, &QLineEdit::textChanged, [filterTimer](QString) { filterTimer->start(); } );
  connect(filterTimer, &QTimer::timeout, this, onFilterTimerTimeout);
}

ProductPickerDialog::~ProductPickerDialog() { delete ui; }

void ProductPickerDialog::on_productView_activated(const QModelIndex& i) {
  if (!i.isValid()) return;
  emit productPicked(i.siblingAtColumn(0).data().toInt());
}
void ProductPickerDialog::on_productView_clicked(const QModelIndex& i)
{
  if (!i.isValid()) return;
  emit productPicked(i.siblingAtColumn(0).data().toInt());
}
void ProductPickerDialog::onFilterTimerTimeout() {
  auto pr = qobject_cast<QSortFilterProxyModel*>(ui->productView->model());
  if (!pr) return ;
  pr->setFilterFixedString(ui->lineEdit->text());
}