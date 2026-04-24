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

void RolesComboBox::setCurrentRoleId(int roleId) {
  auto ilist = qmodel->match(qmodel->index(0,0), Qt::DisplayRole, roleId, 1, Qt::MatchExactly);
  if (!ilist.isEmpty()) {
    setCurrentIndex(ilist.first().row());
    return ;
  }
  setCurrentIndex(-1);
}