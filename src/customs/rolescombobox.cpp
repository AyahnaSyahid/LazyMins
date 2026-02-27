#include "rolescombobox.h"

#include <QSqlQueryModel>
#include <QTableView>
#include <QHeaderView>
#include "src/database/databasemanager.h"

namespace {
  const QString queryText("SELECT id AS ID, role_name AS Peran, description AS Keterangan FROM roles");
}

RolesComboBox::RolesComboBox(QWidget *p): 
QueryComboBox(p)
{
  setQuery(queryText);
  boxView->hideColumn(0);
  boxViewAutoResize();
}

int RolesComboBox::currentRoleId() const {
  return qmodel->index(currentIndex(), 0).data(Qt::EditRole).toInt();
};