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
    
    bool passwordMatch(const QString& user, const QString& pass);
    bool setCurrentAdmin(const QString& user);
    bool resetAdmin();
    // return true, jika memang ada currentAdmin, lainnya false
    
    QString generateSalt(int charCount=32) const;
    QString generateHash(const QString& password, const QString& salt, int iteration=12000) const;
    
    inline const QString &passwordMatchError() const { return m_passwordMatchError; }
    inline const QString &setCurrentAdminError() const { return m_setCurrentAdminError; }
    inline const QSqlRecord &currentAdmin() const { return m_currentAdmin; }

  private:
    AuthManager();
    ~AuthManager();
    
    QSqlRecord m_currentAdmin;
    QString m_passwordMatchError;
    QString m_setCurrentAdminError;
};

#endif