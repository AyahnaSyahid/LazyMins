#include "databasemanager.h"
#include "initializeschema.h"
#include "src/utils/authmanager.h"

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

  if (!db.open()) return false;

  if (!initSchema(db)) return false;

  // Insert super_user — sesuaikan query dengan skema tabel Anda
  auto &am = AuthManager::instance();
  auto salt = am.generateSalt();
  auto hash = am.generateHash(password, salt);
  QSqlQuery q(db);
  q.prepare("INSERT INTO users (username, password, salt, role_id) VALUES (?, ?, ?, 1)");
  q.addBindValue(superUser);
  q.addBindValue(password);
  q.addBindValue(salt);
  
  if (!q.exec()) return false;

  // Simpan path ke settings hanya jika semua berhasil
  QSettings settings;
  settings.setValue("Database/databasePath", dbPath);

  m_database = db;
  m_databaseReady = true;
  m_isFirstRun = false;

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