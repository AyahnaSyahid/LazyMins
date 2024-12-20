#ifndef RoleItemH
#define RoleItemH

#include "databaseitem.h"
#include <QString>

class RoleItem : public DatabaseItem
{
    public:
        implementDatabaseItem;
        
        RoleItem(qint64 role_id=-1);
        RoleItem(const QString& role_name);
        
        // field
        qint64 role_id;
        QString role_name, description;
        tableNameAssign(roles);
        bool operator==(const DatabaseItem&) const override;
};

#endif // RoleItemH