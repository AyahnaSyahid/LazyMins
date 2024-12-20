#include "rolepermissionsitem.h"
#include <QVariant>
// #include <QSqlError>
// #include <QtDebug>

RolePermissionsItem::RolePermissionsItem(const RoleItem& rl) :
    role(rl), permissions(), DatabaseItem()
{
    // qDebug() << role.role_name;
    q.prepare("SELECT role_id, permission_id FROM role_permissions WHERE role_id = ?");
    q.addBindValue(role.role_id);
    q.exec();
    while(q.next()) {
        permissions << q.value(1).toLongLong();
    }
}

RolePermissionsItem::RolePermissionsItem(const QString& role) : RolePermissionsItem(RoleItem(role)) {}

void RolePermissionsItem::setPermission(const PermissionItem& p, bool state)
{
    q.prepare("SELECT permission_id FROM role_permissions "
                "WHERE (permission_id, role_id) = (?, ?); ");
    q.addBindValue(p.permission_id);
    q.addBindValue(role.role_id);
    if(q.exec() && q.next()) {
        if(!state) {
            q.prepare("DELETE FROM role_permissions "
                        "WHERE (role_id, permission_id) = (?, ?);");
            q.addBindValue(role.role_id);
            q.addBindValue(p.permission_id);
            q.exec();
            
            // if(q.lastError().isValid()) {
                // qDebug() << "SET PERM STATE : " << state << "\n";
                // qDebug() << q.lastError().text();
            // }
            
            *this = RolePermissionsItem(role);
        }
    } else {
        if(state) {
            q.prepare("INSERT INTO role_permissions "
                        "(role_id, permission_id, assigned_at) "
                            "VALUES (?, ?, CURRENT_TIMESTAMP);");
            q.addBindValue(role.role_id);
            // qDebug() << role.role_id << role.role_name;
            // qDebug() << p.permission_id << p.permission_name;
            q.addBindValue(p.permission_id);
            q.exec();
            
            // if(q.lastError().isValid()) {
                // qDebug() << "SET PERM STATE : " << state << "\n";
                // qDebug() << q.lastError().text();
            // }
            
            *this = RolePermissionsItem(role);
        }
    }
    // if(q.lastError().isValid()) {
        // qDebug() << "SET PERM LAST\n" << q.lastError().text();
    // }
}

bool RolePermissionsItem::save()
{
    return true;
}

bool RolePermissionsItem::erase()
{
    q.prepare("DELETE FROM role_permissions "
                "WHERE role_id = ?;");
    q.addBindValue(role.role_id);
    q.exec();
    *this = RolePermissionsItem(role);
    return q.numRowsAffected();
}

bool RolePermissionsItem::operator==(const DatabaseItem& dh) const
{
    auto ptr = dynamic_cast<const RolePermissionsItem*>(&dh);
    if(!ptr) return false;
    if(role != ptr->role) return false;
    if(permissions.count() != ptr->permissions.count()) return false;
    for(int c=0; c<permissions.count(); ++c) {
        if(permissions[c] != ptr->permissions[c]) return false;
    }
    return true;
}