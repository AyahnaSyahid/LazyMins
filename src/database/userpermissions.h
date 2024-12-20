#ifndef UserPermissionsH
#define UserPermissionsH

#include "useritem.h"
#include "permissionitem.h"

class UserPermissions
{
    public:
        static bool hasPermission(const UserItem& user, const PermissionItem& perm);
        static bool hasPermission(const PermissionItem& perm);
};

#endif // UserPermissionsH