#ifndef DevModeH
#define DevModeH

#include "useritem.h"
#include "roleitem.h"
#include "permissionitem.h"
#include "rolepermissionsitem.h"
#include "userrolesitem.h"
#include <QSqlQuery>
#include <QtDebug>

namespace DB_INIT {
    void fillDummyDatabaseData() {
#ifdef DEV_MODE_ON

        RoleItem role("Owner");
        if(role.role_id < 1) role.save();
        
        role = RoleItem("Adimistrator");
        if(role.role_id < 1) role.save();
        
        role = RoleItem("UserManager");
        if(role.role_id < 1) role.save();
        
        role = RoleItem("Supervisor");
        if(role.role_id < 1) role.save();
        
        role = RoleItem("Cashier");
        if(role.role_id < 1) role.save();
        
        role = RoleItem("Employee");
        if(role.role_id < 1) role.save();
        
        // Check permissions
        PermissionItem pi("GRANT_ALL");
        if(pi.permission_id < 1) pi.save();
        pi = PermissionItem("ManageUser");
        if(pi.permission_id < 1) pi.save();
        pi = PermissionItem("ManageOrder");
        if(pi.permission_id < 1) pi.save();
        pi = PermissionItem("ManageCustomer");
        if(pi.permission_id < 1) pi.save();
        pi = PermissionItem("AddOrder");
        if(pi.permission_id < 1) pi.save();
        pi = PermissionItem("ManageExpenses");
        if(pi.permission_id < 1) pi.save();
        pi = PermissionItem("Viewer");
        if(pi.permission_id < 1) pi.save();
        pi = PermissionItem("ManageInvoices");
        if(pi.permission_id < 1) pi.save();


        RolePermissionsItem rpi("Owner");
        rpi.setPermission(PermissionItem("AddOrder"), true);
        rpi.setPermission(PermissionItem("ManageUser"), true);
        rpi.setPermission(PermissionItem("ManageOrder"), true);
        rpi.setPermission(PermissionItem("ManageCustomer"), true);
        rpi.setPermission(PermissionItem("ManageExpenses"), true);
        rpi.setPermission(PermissionItem("ManageInvoices"), true);
        rpi.setPermission(PermissionItem("Viewer"), true);
        
        rpi = RolePermissionsItem("Adimistrator");
        rpi.setPermission(PermissionItem("GRANT_ALL"), true);
        
        rpi = RolePermissionsItem("Supervisor");
        rpi.setPermission(PermissionItem("AddOrder"), true);
        rpi.setPermission(PermissionItem("ManageUser"), true);
        rpi.setPermission(PermissionItem("ManageOrder"), true);
        rpi.setPermission(PermissionItem("ManageCustomer"), true);
        rpi.setPermission(PermissionItem("ManageExpenses"), true);
        rpi.setPermission(PermissionItem("ManageInvoices"), true);
        rpi.setPermission(PermissionItem("Viewer"), true);
        
        rpi = RolePermissionsItem("UserManager");
        rpi.setPermission(PermissionItem("ManageCustomer"), true);
        rpi.setPermission(PermissionItem("ManageUser"), true);
        rpi.setPermission(PermissionItem("Viewer"), true);
        
        rpi = RolePermissionsItem("Cashier");
        rpi.setPermission(PermissionItem("ManageCustomer"), true);
        rpi.setPermission(PermissionItem("ManageExpenses"), true);
        rpi.setPermission(PermissionItem("AddOrder"), true);
        rpi.setPermission(PermissionItem("Viewer"), true);
        
        rpi = RolePermissionsItem("Employee");
        rpi.setPermission(PermissionItem("Viewer"), true);
        // rpi.setPermission(PermissionItem("ManageExpenses"), true);
        
        UserItem root(1);
        if(root.user_id < 1) {
            root = UserItem("root");
            root.password = "000000";
            root.save();
        }
        
        UserItem owner(1);
        if(owner.user_id < 1) {
            owner = UserItem("owner");
            owner.password = "000000";
            owner.save();
        }
        
        UserItem spv("Supervisor");
        if(spv.user_id  < 1) {
            spv.password = "123456";
            spv.full_name = "Supervisor One";
            spv.save();
        }
        
        UserItem adm1("Admin1");
        if(adm1.user_id  < 1) {
            adm1.password = "123456";
            adm1.full_name = "Admin One";
            adm1.save();
        }
        
        UserItem adm2("Admin2");
        if(adm2.user_id  < 1) {
            adm2.password = "123456";
            adm2.full_name = "Admin Two";
            adm2.save();
        }
        
        UserItem viewer("Dummy");
        if(viewer.user_id < 1) {
            viewer.password = "000000";
            viewer.full_name = "Test Viewer";
            viewer.save();
        }

        UserRolesItem uri(root);
        uri.setRole("Adimistrator");
        uri = UserRolesItem(owner);
        uri.setRole("Owner", true);
        uri = UserRolesItem(spv);
        uri.setRole("UserManager", true);
        uri.setRole("Cashier", true);
        uri = UserRolesItem(adm1);
        uri.setRole("Cashier", true);
        uri = UserRolesItem(adm2);
        uri.setRole("Cashier", true);
        uri = UserRolesItem(viewer);
        uri.setRole("Employee", true);
#endif
    }
}

#endif // DevModeH