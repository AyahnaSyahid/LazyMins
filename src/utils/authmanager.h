#ifndef AuthManager_H
#define AuthManager_H

#include <QObject>
#include <QSqlRecord>

class AuthManager : public QObject {
    Q_OBJECT
  
  public:
    static AuthManager &instance();
    
    AuthManager(const AuthManager&) = delete;
    AuthManager &operator=(AuthManager&) = delete;
    
    QString generateSalt(int charCount=32) const;
    QString generateHash(const QString& password, const QString& salt, int iteration=12000) const;

  private:
    AuthManager() = default;
    ~AuthManager() = default;
    AuthManager(AuthManager&&) = delete;
    AuthManager &operator=(AuthManager&&) = delete;
};

#endif