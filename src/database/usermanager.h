#ifndef UserManager_H
#define UserManager_H

#include <QObject>

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
    QByteArray generateHash(const QString& s);
    QString generateSalt(unsigned int length=0);

public slots:
    void logout();

private:
    int _c_id;

signals:
    void userLoggedIn(int);
    void userCreated(int);
    void userLoggedOut()

};

#endif