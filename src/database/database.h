#ifndef Database_H
#define Database_H

#ifndef USER_ACCOUNT_DEFAULT_PASSWORD
#define USER_ACCOUNT_DEFAULT_PASSWORD "000000"
#endif

#include <QObject>

class Database : public QObject
{
    Q_OBJECT
public:
    Database(QObject* = nullptr);
    ~Database();
};

#endif