#include "permissiondialog.h"
#include "files/ui_permissiondialog.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>
#include <QHeaderView>

enum QueryDataRole {
  QueryID = Qt::UserRole + 1,
  QueryDescription
};


PermissionDialog::PermissionDialog(int user_id, QWidget *parent)
: _user_id(user_id), 
  permissionModel(new QStandardItemModel(this)),
  ui(new Ui::PermissionDialog),
  _safe_to_open(false),
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
    sim->appendRow(item);
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
}

PermissionDialog::~PermissionDialog() {}

void PermissionDialog::on_comboBox_currentIndexChanged(int ix) {
  auto sim = qobject_cast<QStandardItemModel*>(permissionModel);
  int uid = ui->comboBox->model()->index(ix, 0).data().toInt();
  
  for(int i=1; i < sim->rowCount(); ++i) {
    // i == 0 adalah GRANT_EVERYTHING skip aja
    auto item = sim->item(i);
    item->setCheckState(Qt::Unchecked);
  }
  
  // get user permission list
  QSqlQuery q;
  q.prepare("SELECT user_id, name FROM users_permissions JOIN permissions USING(permission_id) WHERE user_id = ?");
  q.addBindValue(uid);
  q.exec();
  while (q.next()) {
    auto itemList = sim->findItems(q.value("name").toString());
    for(auto item_ptr = itemList.begin(); item_ptr != itemList.end(); ++item_ptr) {
      // (*item_ptr)->text() == q.value("name").toString();
      (*item_ptr)->setCheckState(Qt::Checked);
    }
  }
}

void PermissionDialog::on_itemChanged(QStandardItem *item) {
  
}