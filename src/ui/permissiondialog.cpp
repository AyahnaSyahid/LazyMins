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
#include <QSqlError>
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
  comboView->horizontalHeader()->setStretchLastSection(true);
  comboView->setSelectionBehavior(QAbstractItemView::SelectRows);
  comboModel->setHeaderData(1, Qt::Horizontal, "Nama", Qt::DisplayRole);
  comboModel->setHeaderData(4, Qt::Horizontal, "Alias", Qt::DisplayRole);
  comboView->resizeColumnsToContents();
  comboView->setMinimumWidth(comboView->horizontalHeader()->length());
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
  if(isDirty()) {
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
  // here we load permissions for given userId
  int uid = ui->comboBox->model()->index(ix, 0).data().toInt();
  fetchPermissions(uid);
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
          item->setBackground(QBrush(QColor(181, 255, 198)));
        } else {
          item->setBackground(QBrush(Qt::NoBrush));
        }
      }
    }
  }
}

bool PermissionDialog::isDirty() const {
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  for(int r=0; r < sim->rowCount(); ++r) {
    auto index = sim->itemFromIndex(sim->index(r, 0));
    if(index->background().style() != Qt::NoBrush) {
      return true;
    }
  }
  return false;
}

void PermissionDialog::on_saveButton_clicked() {
  if(isDirty()) {
    // cari item mana yang berubah
    QList<int> removed;
    QList<int> granted;
    for(int ix=0; ix < permissionModel->rowCount(); ++ix) {
       auto iix = permissionModel->index(ix, 0);
       if(iix.data(Qt::BackgroundRole).value<QBrush>().style() != Qt::NoBrush) {
         if(iix.data(Qt::CheckStateRole).value<int>() == Qt::Checked) {
           granted.append(iix.data(QueryID).toInt());
         } else {
           removed.append(iix.data(QueryID).toInt());
         }
       }
    }
    int comboIndex = ui->comboBox->currentIndex();
    int userId = ui->comboBox->model()->index(comboIndex, 0).data(Qt::EditRole).toInt();
    QSqlQuery q("BEGIN TRANSACTION;");
    for(auto gr = granted.cbegin(); gr != granted.cend(); ++gr) {
      q.prepare("INSERT INTO users_permissions (user_id, permission_id) VALUES (?, ?);");
      q.addBindValue(userId);
      q.addBindValue(*gr);
      q.exec();
    }
    for(auto gr = removed.cbegin(); gr != removed.cend(); ++gr) {
      q.prepare("DELETE FROM users_permissions WHERE (user_id, permission_id) = (?, ?);");
      q.addBindValue(userId);
      q.addBindValue(*gr);
      q.exec();
    }
    if(q.exec("COMMIT;")) {
      fetchPermissions(userId);
    } else {
      q.exec("ROLLBACK;");
      QMessageBox::information(this, "Gagal Menyimpan Perubahan", q.lastError().text());
    }
  }
}

void PermissionDialog::fetchPermissions(int uid) {
  
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
  
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  
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
}