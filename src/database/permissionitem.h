#ifndef PermissionItemH
#define PermissionItemH

#include "databaseitem.h"
#include <QString>

class PermissionItem : public DatabaseItem
{
    public:
        qint64 permission_id;
        QString permission_name, description;
        
        implementDatabaseItem;
        PermissionItem(qint64 = -1);
        PermissionItem(const QString& nn);
        tableNameAssign(permissions);
        bool operator==(const DatabaseItem&) const override;
};

#endif // PermissionItemH