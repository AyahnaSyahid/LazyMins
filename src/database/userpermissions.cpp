#include "userpermissions.h"
bool UserPermissions::hasPermission(const UserItem& u, const PermissionItem& p)
{
    QSqlQuery q;
    q.prepare(R"-(
       SELECT user_roles.user_id,
           users.username,
           role_permissions.permission_id,
           permissions.permission_name,
           roles.role_name
      FROM user_roles,
           users USING (user_id),
           roles USING (role_id),
           role_permissions USING (role_id),
           permissions USING (permission_id)
    WHERE  ( permissions.permission_name = ? OR permissions.permission_name = "GRANT_ALL" ) AND user_id = ?; )-");
    q.addBindValue(p.permission_name);
    q.addBindValue(u.user_id);
    return q.exec() && q.next();
}

namespace GlobalNamespace {
extern qint64 logged_user_id;
}

bool UserPermissions::hasPermission(const PermissionItem& perm) {
    UserItem ui(GlobalNamespace::logged_user_id);
    if(ui.user_id < 1) return false;
    return hasPermission(ui, perm);
}