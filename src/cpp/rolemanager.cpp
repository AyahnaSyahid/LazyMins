#include "rolemanager.h"
#include <QSqlQuery>

bool RoleManager::createRole(const QString& name, const QString& desc, Role *ref) {
    QSqlQuery q;
    q.prepare("SELECT role_id, role_name, description FROM roles WHERE role_name = ?");
    q.addBindValue(name);
    q.exec();
    if(q.next()) {
        qint64 role_id = q.value(0).toLongLong();
        if(desc != q.value(2).toString()) {
            q.prepare("UPDATE roles SET description = ? WHERE role_id = ?;");
            q.addBindValue(desc);
            q.addBindValue(role_id);
            q.exec();
            if(ref) {
                ref->role_id = role_id;
                ref->description = desc;
                ref->role_name = name;
            }
            return true;
        }
    } else {
        q.prepare("INSERT INTO roles (role_name, description) VALUES (?, ?)");
        q.addBindValue(name);
        q.addBindValue(desc);
        if(q.exec()) {
            if(ref) {
                ref->role_id = q.lastInsertId().toLongLong();
                ref->role_name = name;
                ref->description = desc;
            }
            return true;
        }
    }
    return false;
}

Role RoleManager::getRole(qint64 role_id) const {
    QSqlQuery q;
    q.prepare("SELECT role_id, role_name, description FROM roles WHERE role_id = ?");
    q.addBindValue(role_id);
    if(q.exec() && q.next()) {
        return Role::fromRecord(q.record());
    }
    return Role();
}

Role RoleManager::removeRole(qint64 role_id) {
    QSqlQuery q;
    Role r;
    q.prepare("SELECT role_id, role_name, description FROM roles WHERE role_id = ?");
    q.addBindValue(role_id);
    if(q.exec() && q.next()) {
        r = Role { q.value(0).toLongLong(), q.value(1).toString(), q.value(2).toString() };
    }
    q.prepare("DELETE FROM roles WHERE role_id = ?");
    q.addBindValue(role_id);
    q.exec();
    return r;
}
