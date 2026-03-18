#include "authmanager.h"
#include "src/database/databasemanager.h"
#include "src/managers/adminmanager.h"
#include <QRandomGenerator>
#include <QPasswordDigestor>
#include <QCryptographicHash>
#include <QByteArray>
#include <QSqlQuery>
#include <QSqlError>


AuthManager::AuthManager() : m_currentAdmin()
{}

AuthManager::~AuthManager() {}

AuthManager &AuthManager::instance()
{
  static AuthManager am;
  return am;
}

QString AuthManager::generateSalt(int charCount) const 
{
    // Kumpulan karakter yang akan digunakan untuk salt
    // Menggunakan Alphanumeric (A-Z, a-z, 0-9)
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    
    QString randomString;
    randomString.reserve(charCount); // Optimasi memori agar tidak resize berulang kali

    for(int i = 0; i < charCount; ++i)
    {
        // Mengambil index acak dari rentang 0 sampai panjang possibleCharacters - 1
        quint32 index = QRandomGenerator::global()->bounded(static_cast<quint32>(possibleCharacters.length()));
        randomString.append(possibleCharacters.at(index));
    }

    return randomString;
}

QString AuthManager::generateHash(const QString& password, const QString& salt, int iteration) const {
  int dklen= 32; 
  QByteArray derivedKey = QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha256, 
        password.toUtf8(), 
        salt.toUtf8(), 
        iteration, 
        dklen
    );
  return derivedKey.toHex();
}

bool AuthManager::passwordMatch(const QString& user, const QString& password)
{
  m_passwordMatchError = "";
  auto orec = AdminManager().getRecord(user);
  if (orec) {
    auto rec = *orec;
    // qDebug() << rec;
    auto hash = generateHash(password, rec.value("salt").toString());
    // qDebug() << hash;
    if (hash == rec.value("password_hash").toString()) {
      if (rec.value("is_active").toInt() != 1) {
        m_passwordMatchError = "Access denied: User status is inactive";
        return false;
      }
      return true;
    }
    m_passwordMatchError = "Access denied: Username or Password missmatch";
  } else {
    m_passwordMatchError = "Access denied: unregistered Username";
  }
  return false;
}

bool AuthManager::setCurrentAdmin(const QString& user) {
  auto &db = DatabaseManager::instance();
  if (!db.isOpen()) return false;
  QSqlQuery query(db.database());
  query.prepare("SELECT * FROM admins WHERE username = :username");
  query.bindValue(":username", user);
  if (query.exec() && query.next()) {
    m_currentAdmin = query.record();
    return true;
  }
  return false;
}

bool AuthManager::resetAdmin() {
  if(! m_currentAdmin.isEmpty() ) {
    m_currentAdmin.clear();
    return true;
  }
  return false;
}