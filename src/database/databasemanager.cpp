#include "databasemanager.h"
#include "initializeschema.h"
#include "src/utils/authmanager.h"
#include "src/managers/basemanager.h"

#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

DatabaseManager &DatabaseManager::instance() {
  static DatabaseManager dbm;
  return dbm;
}

bool DatabaseManager::initializeFromSetup(const QString &dbPath,
                                          const QString &superUser,
                                          const QString &password)
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "LMAdmins_db");
  db.setDatabaseName(dbPath);

  if (!db.open()) {
    qWarning() << "[DatabaseManager::initializeFromSetup] Database open failed:" << db.lastError().text();
    db.close();
    return false;
  };

  if (!initSchema(db)) {
    qWarning() << "[DatabaseManager::initializeFromSetup] Schema initialization failed:" << db.lastError().text(); 
    db.close(); 
    return false;
  };

  // Insert super_user — sesuaikan query dengan skema tabel Anda
  auto &am = AuthManager::instance();
  auto salt = am.generateSalt();
  auto hash = am.generateHash(password, salt);
  QSqlQuery q(db);
  qInfo() << "[DatabaseManager::initializeFromSetup] databaseConnection:" << db.connectionName();
  qInfo() << "[DatabaseManager::initializeFromSetup] databaseName:" << db.databaseName();
  q.prepare("INSERT INTO admins (username, password_hash, nama_lengkap, salt, role_id) VALUES (:un, :pw, :nl, :st, 1)");
  q.bindValue(":un", superUser);
  q.bindValue(":pw", hash);
  q.bindValue(":nl", superUser);
  q.bindValue(":st", salt);
  
  if (!q.exec()) {
    qWarning() << "[DatabaseManager::initializeFromSetup] Super user creation failed:" << q.lastError().text();
    return false;
  };

  // Simpan path ke settings hanya jika semua berhasil
  QSettings settings;
  settings.setValue("Database/databasePath", dbPath);

  m_database = db;
  m_databaseReady = true;
  m_isFirstRun = false;

  settings.sync();
  return true;
}

void DatabaseManager::setDatabase(QSqlDatabase &db)
{
    m_database = db;
    QSqlQuery q("PRAGMA foreign_keys = ON;", db);
}

DatabaseManager::DatabaseManager() : m_databaseReady(false), m_isFirstRun(false)
{
  QSettings settings;

  if (!settings.contains("Database/databasePath")) {
    m_isFirstRun = true;
    return;  // berhenti di sini, tunggu initializeFromSetup()
  }

  QString dbPath = settings.value("Database/databasePath").toString();

  QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE", "LMAdmins_db");
  _db.setDatabaseName(dbPath);

  if (!_db.open()) {
    m_databaseReady = false;
    return;
  }

  if (!verifySchema(_db)) {
    m_databaseReady = false;
    return;
  }

  m_database = _db;
  m_databaseReady = true;
  BaseManager::connection = m_database;
}

bool DatabaseManager::initSchema(QSqlDatabase &db) {
  return initializeSchemaFile(":/schema/main.sql", db);
};

DatabaseManager::~DatabaseManager()
{
  if(m_database.isValid() && m_database.isOpen()) {
    m_database.close();
  }
}

bool DatabaseManager::isOpen() const {
  return m_database.isOpen();
}

QSqlError DatabaseManager::lastError() const
{
  return m_database.lastError();
}

bool DatabaseManager::verifySchema(QSqlDatabase &db) {
  return true;
}