#ifndef UserRoleManagerH
#define UserRoleManagerH

#include "usermanager.h"
#include "rolemanager.h"

class UserRoleManager {
    public:
        void setUserRole(qint64 user_id, qint64 role_id, bool state = true);
        void setUserRole(const User& user, const Role& role, bool state = true);
}

#endif //UserRolesManagerH