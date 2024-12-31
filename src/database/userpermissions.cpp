#include "userpermissions.h"
bool UserPermissions::hasPermission(const UserItem& u, const PermissionItem& p)
{
    QSqlQuery q;
    q.prepare(R"-(
SELECT * FROM ( SELECT user_roles.user_id,
           users.username,
           role_permissions.permission_id,
           permissions.permission_name,
           roles.role_name
      FROM user_roles,
           users USING (user_id),
           roles USING (role_id),
           role_permissions USING (role_id),
           permissions USING (permission_id)
           UNION ALL 
               SELECT user_id, 
                      NULL, 
                      permission_id, 
                      permission_name, 
                      NULL 
                   FROM user_permissions 
                       JOIN permissions USING (permission_id) ) P
           WHERE ( permission_name = ? OR permission_name = "GRANT_ALL" ) AND user_id = ?;
    )-");
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

bool UserPermissions::setPermission(const UserItem& user, const PermissionItem& perm, bool allow) {
    // this only manage user_permissions table, not for role_permissions
    QSqlQuery q;
    bool _hasPerm = hasPermission(user, perm);
    if (allow){
        if(!_hasPerm) {
            q.prepare("INSERT INTO user_permissions (user_id, permission_id) VALUES (?, ?)");
            q.addBindValue(user.user_id);
            q.addBindValue(perm.permission_id);
            return q.exec();
        } else {
            return true;
        }
    } else {
        // we can only disable permission which is not part of role_permissions
        if(_hasPerm) {
            q.prepare("DELETE FROM user_permissions WHERE user_id = ? AND permission_id = ?");
            q.addBindValue(user.user_id);
            q.addBindValue(perm.permission_id);
            return q.exec();
        }
    }
    return true;
}