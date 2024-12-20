#include "roleitem.h"
#include <QVariant>

RoleItem::RoleItem(qint64 id) :
    role_id(id),
    DatabaseItem()
{
    if(role_id > 0) {
        q.prepare("SELECT role_id, role_name, description FROM roles WHERE role_id = ?");
        q.addBindValue(role_id);
        if(q.exec() && q.next()) {
            qValueAssign(role_name, QString)
            qValueAssign(description, QString)
        } else {
            role_id = -1;
        }
    }
}

RoleItem::RoleItem(const QString& rname) : role_id(-1), role_name(rname), DatabaseItem() {
    
    q.prepare("SELECT role_id, role_name, description FROM roles WHERE role_name = ?");
    q.addBindValue(rname);
    
    if(q.exec() && q.next()) {
        qValueAssign(role_id, qint64)
        qValueAssign(role_name, QString)
        qValueAssign(description, QString)
    } else {
        role_id = -1;
    }
}

bool RoleItem::save() {
    if(role_name.isEmpty()) return false;
    if(role_id < 1) {
        // create
        q.prepare(R"_(INSERT INTO roles (role_name, description)
                        VALUES (?, ?); )_");
        q.addBindValue(role_name);
        q.addBindValue(description);
        if(q.exec()) {
            role_id = q.lastInsertId().toLongLong();
            return true;
        }
    } else {
        q.prepare(R"_(
            UPDATE roles SET (role_name, description) =
                (?, ?) WHERE role_id = ?;
        )_");
        q.addBindValue(role_name);
        q.addBindValue(description);
        q.addBindValue(role_id);
        if(q.exec()) {
            if(q.numRowsAffected()) {
                return true;
            }
        }
    }
    return false;
}

bool RoleItem::erase() {
    if(role_id < 1) return false;
    q.prepare("DELETE FROM roles WHERE role_id = ?");
    q.addBindValue(role_id);
    if(q.exec() && q.numRowsAffected()) {
        role_id = -1;
    }
    return false;
}

bool RoleItem::operator==(const DatabaseItem& ri) const {
    const RoleItem* pri = dynamic_cast<const RoleItem*>(&ri);
    if(!pri) return false;
    return role_id == pri->role_id &&
           role_name == pri->role_name &&
           description == pri->description;
}