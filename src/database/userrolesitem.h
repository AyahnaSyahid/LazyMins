#ifndef UserRolesItemH
#define UserRolesItemH

#include "roleitem.h"
#include "useritem.h"

#include <QList>

class UserRolesItem : public DatabaseItem{
    UserItem user;
  public:
    QList<RoleItem> roles;
    void setRole(const RoleItem& role, bool =true);
    void setRole(const QString& srole, bool state=true) { setRole(RoleItem(srole), state); }
    
    inline const UserItem& getUser() const { return user; }
    inline UserItem& getUser() { return user; }
    
    UserRolesItem(const QString& suser) : UserRolesItem(UserItem(suser)) {}
    UserRolesItem(const UserItem& user);
    
    implementDatabaseItem;
    tableNameAssign(user_roles);
    bool operator==(const DatabaseItem&) const override;
};

#endif // UserRolesItemH