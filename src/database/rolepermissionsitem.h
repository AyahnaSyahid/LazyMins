#ifndef RolePermissionsItemH
#define RolePermissionsItemH

#include "roleitem.h"
#include "permissionitem.h"

#include <QList>

class RolePermissionsItem : public DatabaseItem
{
    RoleItem role;
  public:
    QList<PermissionItem> permissions;
    RolePermissionsItem(const RoleItem& role);
    RolePermissionsItem(const QString& role);
    
    void setPermission(const PermissionItem& p, bool state=true);
    inline const RoleItem& getRole() const { return role; }
    inline RoleItem& getRole() { return role; }
    
    implementDatabaseItem;
    tableNameAssign(role_permissions);
    bool operator==(const DatabaseItem&) const override;
};


#endif // RolePermissionItemH