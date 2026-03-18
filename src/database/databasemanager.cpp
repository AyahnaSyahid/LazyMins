#include "databasemanager.h"
#include "initializeschema.h"

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

void DatabaseManager::setDatabase(QSqlDatabase &db) {
  m_database = db;
}

DatabaseManager::DatabaseManager() : m_databaseReady(false)
{
  QSettings settings;
  QString dbPath = ":memory:";
  if (settings.contains("Database/databasePath")) {
    dbPath = settings.value("Database/databasePath").toString();
  }
  
  QSqlDatabase _db = QSqlDatabase::database("LMAdmins_db", true);
  
  if (!_db.isValid()) {
    _db = QSqlDatabase::addDatabase("QSQLITE", "LMAdmins_db");
    if (dbPath != ":memory:") {
      if (!QFileInfo::exists(dbPath)) {
        // create new database file
        _db.setDatabaseName(dbPath);
        if (!_db.open()) {
          m_databaseReady = false;
        } else {
          if( !initSchema(_db)) {
            m_databaseReady = false;
          } else {
            if(verifySchema(_db)) {
              m_databaseReady = true;
            } else {
              m_databaseReady = false;
            }
          }
        }
      } else {
        _db.setDatabaseName(dbPath);
        if(verifySchema(_db)) {
          m_databaseReady = true;
        } else {
          m_databaseReady = false;
        }
      }
    } else {
      _db.setDatabaseName(":memory:");
      _db.open();
      if (!initSchema(_db)) {
        m_databaseReady = false;
      } else {
        if (verifySchema(_db)) {
          m_databaseReady = true;
        } else {
          m_databaseReady = false;
        }
      }
    }
  } else {
    if (_db.tables().count() < 5) {
      initSchema(_db);
    }
    m_databaseReady = true;
  }
  
  if (m_databaseReady) {
    m_database = _db;
  }
};

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