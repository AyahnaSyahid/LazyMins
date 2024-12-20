#include "permissionitem.h"
#include <QVariant>

PermissionItem::PermissionItem(qint64 pid) : 
    permission_id(pid), DatabaseItem()
{
    if(pid > 0) {
        q.prepare(R"_(
            SELECT permission_id,
                   permission_name,
                   description
              FROM permissions WHERE permission_id = ?; )_");

        q.addBindValue(pid);
        if(q.exec() && q.next()) {
            qValueAssign(permission_id, qint64);
            qValueAssign(permission_name, QString);
            qValueAssign(description, QString);
        }
    }
}

PermissionItem::PermissionItem(const QString& nn) :
    permission_name(nn),
    permission_id(-1)
{
    q.prepare(R"_(
        SELECT permission_id,
               permission_name,
               description
          FROM permissions WHERE permission_name = ?; )_");
    q.addBindValue(nn);
    if(q.exec() && q.next()) {
        qValueAssign(permission_id, qint64);
        qValueAssign(description, QString);
    }
}

bool PermissionItem::save() {
    if(permission_id < 1) {
        if(permission_name.isEmpty()) return false;
        q.prepare(R"-(
            INSERT INTO permissions (permission_name, description)
                VALUES (?, ?);
        )-");
        q.addBindValue(permission_name);
        q.addBindValue(description);
        if(q.exec()) {
            permission_id = q.lastInsertId().toLongLong();
            return true;
        }
    } else {
        if(permission_name.isEmpty()) return false;
        q.prepare(R"-(
            UPDATE permissions SET 
                (permission_name, description) =
                (?, ?) WHERE permission_id = ?;
        )-");
        q.addBindValue(permission_name);
        q.addBindValue(description);
        q.addBindValue(permission_id);
        if(q.exec() && q.numRowsAffected())
            return true;
    }
    return false;
}

bool PermissionItem::erase() {
    if(permission_id < 1) return true;
    q.prepare("DELETE FROM permissions WHERE permission_id = ?");
    q.addBindValue(permission_id);
    return q.exec();
}

bool PermissionItem::operator==(const DatabaseItem& ot) const {
    auto ptr = dynamic_cast<const PermissionItem*>(&ot);
    if(!ptr) return false;
    return permission_id == ptr->permission_id &&
           permission_name == ptr->permission_name;
}