#include "databasemanager.h"
#include "src/utils/sqlschemaparser.h"
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

DatabaseManager::DatabaseManager() : m_databaseReady(false)
{
  QSettings settings;
  QString dbPath = ":memory:";
  if (settings.contains("Database/databasePath")) {
    dbPath = settings.value("Database/databasePath").toString();
  }

  QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE", "LMAdmins_db");
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
  if (m_databaseReady) {
    m_database = _db;
  }
};

bool DatabaseManager::initSchema(QSqlDatabase &db) {
  if (!db.isOpen()) return false;
  QStringList stl;
  QFile sf(":/schema/main.sql");
  if (!sf.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  QTextStream ts(&sf);
  bool insideCreateTrigger = false;
  QString statement, line;
  while (!ts.atEnd()) {
    line = ts.readLine();
    if (line.isEmpty()) {
      continue;
    }
    if (line.toLower().contains("create trigger")) {
      insideCreateTrigger = true;
      statement += "\n" + line;
      continue;
    }
    if (line.contains(";")) {
      statement += "\n" + line;
      if (insideCreateTrigger) {
        if (line.toLower().contains("end;")) {
          insideCreateTrigger = false;
          stl << statement;
          statement.clear();
          continue;
        }
        continue;
      } else {
        stl << statement;
        statement.clear();
      }
      continue;
    }
    statement += "\n" + line;
  }
  qDebug() << "Using :" << db.databaseName();
  QSqlQuery q(db);
  for (auto st : stl) {
    qDebug() << st;
    qDebug() << q.exec(st);
  }
  qDebug() <<  db.tables();
  return false;
};

DatabaseManager::~DatabaseManager()
{
  if(m_database.isValid() && m_database.isOpen()) {
    m_database.close();
  }
}

bool DatabaseManager::transaction() {
  return m_database.transaction();
}

bool DatabaseManager::commit() {
  return m_database.commit();
}

bool DatabaseManager::rollback() {
  return m_database.rollback();
}

bool DatabaseManager::isOpen() const {
  return m_database.isOpen();
}

QSqlError DatabaseManager::lastError() const
{
  return m_database.lastError();
}

bool DatabaseManager::verifySchema(QSqlDatabase &db) {
  return false;
}