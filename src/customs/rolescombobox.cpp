#include "rolescombobox.h"

#include <QSqlQueryModel>
#include <QTableView>
#include <QHeaderView>
#include "src/database/databasemanager.h"

namespace {
  const QString queryText("SELECT id AS ID, role_name AS Peran, description AS Keterangan FROM roles");
}

RolesComboBox::RolesComboBox(QWidget *p):
qmodel(new QSqlQueryModel(this)), QComboBox(p)
{
  qmodel->setQuery( queryText, DatabaseManager::instance().database());
  setModel(qmodel);
  setModelColumn(1);
  
  auto boxView = new QTableView();
  setView(boxView);
  setCurrentIndex(2);
  
  auto vh = boxView->verticalHeader();
  vh->setMinimumSectionSize(22);
  vh->setDefaultSectionSize(20);
  vh->hide();
  boxView->horizontalHeader()->hide();
  boxView->hideColumn(0);
  boxView->setAlternatingRowColors(true);
  boxView->resizeColumnsToContents();
  boxView->setHorizontalScrollMode(QTableView::ScrollPerPixel);
  boxView->setSelectionBehavior(QTableView::SelectRows);
  boxView->setMinimumWidth(boxView->columnWidth(1) + boxView->columnWidth(2));
}

int RolesComboBox::currentRoleId() const {
  return qmodel->index(currentIndex(), 0).data(Qt::EditRole).toInt();
}

void RolesComboBox::refetchData() {
  int cix = currentIndex();
  qmodel->setQuery(queryText, DatabaseManager::instance().database());
  setCurrentIndex(cix >= qmodel->rowCount() ? qmodel->rowCount() - 1 : cix);
}