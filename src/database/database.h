#ifndef Database_H
#define Database_H

#ifndef USER_ACCOUNT_DEFAULT_PASSWORD
#define USER_ACCOUNT_DEFAULT_PASSWORD "000000"
#endif

class Database : public QObject
{
    Q_OBJECT

public:
    Database(QObject* = nullptr);
    ~Database();
    bool login(const QString& account, const QString& password);
    bool addCustomer(const QString& name, const QString& address, const QString& phone);    
    bool addUser(const QString& account,
                 const QString& password=USER_ACCOUNT_DEFAULT_PASSWORD,
                 const QString& displayName = "", int * out_userId = nullptr);
    bool addProduct(const QString& name, const QString& desc
};

#endif