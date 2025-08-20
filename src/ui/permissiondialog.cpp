#include "permissiondialog.h"
#include "files/ui_permissiondialog.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>
#include <QHeaderView>
#include <QColor>
#include <QBrush>
#include <QMessageBox>
#include <QtDebug>

enum QueryDataRole {
  QueryID = Qt::UserRole + 1,
  QueryDescription
};

PermissionDialog::PermissionDialog(int user_id, QWidget *parent)
: _user_id(user_id), 
  permissionModel(new QStandardItemModel(this)),
  ui(new Ui::PermissionDialog),
  checkedIndex {},
  QDialog(parent) {
  ui->setupUi(this);
  
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  // get the permission list
  QSqlQuery q("SELECT * FROM permissions");
  while(q.next()) {
    auto item = new QStandardItem(q.value("name").toString());
    item->setData(q.value("permission_id").toInt(), QueryID);
    item->setToolTip(q.value("description").toString());
    item->setEditable(false);
    if(q.value("name").toString() == "GRANT_EVERYTHING") {
      item->setCheckable(false);
      item->setEnabled(false);
    } else {
      item->setCheckable(true);    
    }
    item->setBackground(QBrush(Qt::NoBrush));
    sim->appendRow(item);
    // qDebug() << item->background();
  };
  
  ui->listView->setModel(permissionModel);
  
  auto comboView = new QTableView(this);
  auto comboModel = new QSqlTableModel(this);
  comboModel->setTable("users");
  comboModel->select();
  ui->comboBox->setView(comboView);
  ui->comboBox->setModel(comboModel);
  ui->comboBox->setModelColumn(1);
  comboView->verticalHeader()->hide();
  comboView->hideColumn(0);
  comboView->hideColumn(2);
  comboView->hideColumn(3);
  comboView->hideColumn(5);
  comboView->hideColumn(6);
  comboView->hideColumn(7);
  comboView->verticalHeader()->setMinimumSectionSize(18);
  comboView->verticalHeader()->setDefaultSectionSize(18);
  comboView->resizeColumnsToContents();
  comboView->setMinimumWidth(comboView->horizontalHeader()->length());
  comboView->horizontalHeader()->setStretchLastSection(true);
  comboModel->setHeaderData(1, Qt::Horizontal, "Nama", Qt::DisplayRole);
  comboModel->setHeaderData(4, Qt::Horizontal, "Alias", Qt::DisplayRole);
  setProperty("lastView", -1);
  connect(permissionModel, &QAbstractItemModel::dataChanged, this, &PermissionDialog::on_itemDataChanged);
}

PermissionDialog::~PermissionDialog() {}

void PermissionDialog::on_comboBox_currentIndexChanged(int ix) {
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  int uid = ui->comboBox->model()->index(ix, 0).data().toInt();
  for(int r=0; r < sim->rowCount(); ++r) {
    auto index = sim->itemFromIndex(sim->index(r, 0));
    if(index->background().style() != Qt::NoBrush) {
      auto que = QMessageBox::question(this, "Informasi", "abaikan?");
      if(que == QMessageBox::No) {
        ui->comboBox->setCurrentIndex(property("lastView").toInt());
        return ;
      }
    }
  }
  // get user permission list
  QSqlQuery q;
  q.prepare("SELECT user_id, name FROM users_permissions JOIN permissions USING(permission_id) WHERE user_id = ?");
  q.addBindValue(uid);
  q.exec();
  while (q.next()) {
    auto itemList = sim->findItems(q.value("name").toString());
    for(auto item_ptr = itemList.begin(); item_ptr != itemList.end(); ++item_ptr) {
      (*item_ptr)->setCheckState(Qt::Checked);
    }
  }
  setProperty("lastView", ix);
}

void PermissionDialog::on_itemDataChanged(const QModelIndex& tl, const QModelIndex& bl, const QVector<int> &roles){
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  if(roles.contains(Qt::CheckStateRole)) {
    qDebug() << "on item CheckMenu";
    for(int r = tl.row(); r <= bl.row(); ++r) {
      for(int c = tl.column(); c <= tl.column(); ++c) {
        auto item = sim->itemFromIndex(sim->index(r, c));
        auto brush = item->background();
        if(brush.style() == Qt::NoBrush) {
          item->setBackground(QBrush(QColor(168, 158, 50)));
        } else {
          item->setBackground(QBrush(Qt::NoBrush));
        }
      }
    }
  }
}