#include "authmanager.h"
#include <QRandomGenerator>
#include <QPasswordDigestor>
#include <QCryptographicHash>
#include <QByteArray>
#include <QSqlQuery>
#include <QSqlError>

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
