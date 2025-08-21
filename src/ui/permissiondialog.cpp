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
#include <QComboBox>
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
  connect(permissionModel, &QAbstractItemModel::dataChanged, this, &PermissionDialog::on_itemDataChanged);
  setProperty("lastView", -1);
}

PermissionDialog::~PermissionDialog() {}

void PermissionDialog::on_comboBox_currentIndexChanged(int ix) {
  /* CurrentUser selecting userId
      if there are modification on item inside permissionsModel
        ask for ignoring modification or return for continue editing selected userId
      load permissions to be edited otherwise
  */
  
  // modification checking
  //    when item is modified we mark its backgroud brush
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  for(int r=0; r < sim->rowCount(); ++r) {
    auto index = sim->itemFromIndex(sim->index(r, 0));
    if(index->background().style() != Qt::NoBrush) {
      // item change detected here
      auto que = QMessageBox::question(this, "Data telah diubah", "Lanjut dan abaikan perubahan ?");
      if(que == QMessageBox::No) {
          // currentUser decide to continue editing last userId
          ui->comboBox->blockSignals(true);
          ui->comboBox->setCurrentIndex(property("lastView").toInt());
          ui->comboBox->blockSignals(false);
          // just return dont do anything further
          return ;
      } 
      // currentUser decide to ignoring data modification
    }
  }
  
  // here we load permissions for given userId
  int uid = ui->comboBox->model()->index(ix, 0).data().toInt();

  QSqlQuery q;
  q.prepare(R"--(
      SELECT per.permission_id,
       per.name,
       (up.user_id IS NOT NULL) AS has_permission,
       description
  FROM permissions per
       LEFT JOIN
       users_permissions up ON up.permission_id = per.permission_id AND 
                               up.user_id = ?
  ORDER BY per.permission_id;)--");
  q.addBindValue(uid);
  q.exec();
  
  // Block signal sementara saat menginisialisasi centang pada permissions list
  disconnect(sim, &QAbstractItemModel::dataChanged, this, &PermissionDialog::on_itemDataChanged);
  sim->clear();
  while(q.next()) {
    auto item = new QStandardItem(q.value("name").toString());
    item->setData(q.value("permission_id").toInt(), QueryID);
    item->setToolTip(q.value("description").toString());
    item->setEditable(false);
    if(q.value("name").toString() == "GRANT_EVERYTHING") {
      item->setCheckable(false);
      item->setCheckState(q.value("has_permission").toBool() ? Qt::Checked : Qt::Unchecked);    
      item->setEnabled(false);
    } else {
      item->setCheckable(true);    
      item->setCheckState(q.value("has_permission").toBool() ? Qt::Checked : Qt::Unchecked);    
    }
    sim->appendRow(item);
  }
  connect(sim, &QAbstractItemModel::dataChanged, this, &PermissionDialog::on_itemDataChanged);
  setProperty("lastView", ix);
}

void PermissionDialog::on_itemDataChanged(const QModelIndex& tl, const QModelIndex& bl, const QVector<int> &roles) {
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  if(roles.contains(Qt::CheckStateRole)) {
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

void PermissionDialog::loadPermissionsForUser(int uid) {
  
}