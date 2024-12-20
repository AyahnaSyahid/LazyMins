#ifndef UserItemH
#define UserItemH

#include "databaseitem.h"
#include <QDateTime>

class UserItem : public DatabaseItem
{
  public:
    qint64 user_id;
    QString username,
            salt, 
            password_hash, 
            full_name, 
            home_address, 
            email, 
            phone_number, 
            password;
    
    bool is_active;
    QDateTime created_at, 
              updated_at;
    
    UserItem(qint64 user_id = -1);
    UserItem(const QString& name);
    
    static bool passwordMatch(const UserItem& user, const QString& password);
    
    ~UserItem() {};
    
    implementDatabaseItem;
    tableNameAssign(users);
    bool operator==(const DatabaseItem& other) const override;
};

#endif // UserItemH