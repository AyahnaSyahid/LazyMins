#include "useritem.h"
#include "roleitem.h"
#include "permissionitem.h"
#include "userrolesitem.h"
#include "rolepermissionsitem.h"
#include "customeritem.h"

#include <QtDebug>

int main(int argv, char** args) {
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("seriously.db");
    db.open();
    // qDebug() << "Create empty User";
    UserItem uadmin("nur_holis");
    if(uadmin.user_id < 1)
        uadmin.save();
    CustomerItem ci("Hakim");
    ci.created_by = uadmin;
    ci.save();
    return 0;
}
