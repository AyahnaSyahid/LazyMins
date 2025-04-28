#ifndef UserManager_H
#define UserManager_H

#include <QObject>
#include <QCryptographicHash>
#include <QByteArray>

class UserManager : public QObject {
    Q_OBJECT

public:
    explicit UserManager(QObject* =nullptr);
    ~UserManager();
    
    bool login(const QString& a, const QString& p);
    bool createUser(const QString& a,
                    const QString& p,
                    const QString& d,
                    QString* err=nullptr,
                    int* out_id=nullptr);

    const int& currentUser() const { return _c_id ; }


// STATIC

    static bool nameExists(const QString& name);
    static QByteArray generateHash(const QString& pw, const QString& salt);
    static QString generateSalt(int length=0);

public slots:
    void logout();

private:
    int _c_id;

signals:
    void userLoggedIn(int);
    void userCreated(int);
    void userLoggedOut();

};

#endif