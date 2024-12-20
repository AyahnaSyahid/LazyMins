#include "userrolesitem.h"
#include <QVariant>

UserRolesItem::UserRolesItem(const UserItem& u) :
    user(u), roles()
{
    q.prepare("SELECT role_id FROM user_roles WHERE user_id = ?");
    q.addBindValue(user.user_id);
    if(q.exec()) {
        while(q.next()){
            roles << RoleItem(q.value("role_id").toLongLong());
        }
    }
}

void UserRolesItem::setRole(const RoleItem& role, bool state)
{
    if(role.role_id < 1) return;
    q.prepare("SELECT role_id FROM user_roles WHERE (role_id, user_id) = (?, ?);");
    q.addBindValue(role.role_id);
    q.addBindValue(user.user_id);
    if(q.exec() && q.next()) {
        if(!state) {
            q.prepare("DELETE FROM user_roles WHERE (role_id, user_id ) = (?, ?);");
            q.addBindValue(role.role_id);
            q.addBindValue(user.user_id);
            q.exec();
            *this = UserRolesItem(user);
        }
    } else {
        if(state) {
            q.prepare(R"-(
                INSERT INTO user_roles (user_id, role_id, assigned_at)
                    VALUES (?, ?, CURRENT_TIMESTAMP);
            )-");
            q.addBindValue(user.user_id);
            q.addBindValue(role.role_id);
            q.exec();
            *this = UserRolesItem(user);
        }
    }
}

bool UserRolesItem::save()
{ return true; }

bool UserRolesItem::erase()
{
    q.prepare("DELETE FROM user_roles WHERE user_id = ?");
    q.addBindValue(user.user_id);
    q.exec();
    return q.numRowsAffected();
}

bool UserRolesItem::operator==(const DatabaseItem& ot) const {
    auto pt = dynamic_cast<const UserRolesItem*>(&ot);
    if(!pt) return false;
    if(pt) {
        if(user != pt->user) return false;
        if(roles.count() != pt->roles.count()) return false;
        for(int c=0; c<roles.count(); ++c) {
            if(roles[c] != pt->roles[c]) return false;
        }
    }
    return true;
}