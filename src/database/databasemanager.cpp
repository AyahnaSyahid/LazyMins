#include "databasemanager.h"

#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>

#include "initializeschema.h"
#include "src/managers/basemanager.h"
#include "src/utils/authmanager.h"

DatabaseManager& DatabaseManager::instance() {
  static DatabaseManager dbm;
  return dbm;
}

bool DatabaseManager::initializeFromSetup(const QString& dbPath,
                                          const QString& superUser,
                                          const QString& password) {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "LMAdmins_db");
  db.setDatabaseName(dbPath);

  if (!db.open()) {
    qWarning() << "[DatabaseManager::initializeFromSetup] Database open failed:"
               << db.lastError().text();
    db.close();
    return false;
  };

  if (!initSchema(db)) {
    qWarning() << "[DatabaseManager::initializeFromSetup] Schema "
                  "initialization failed:"
               << db.lastError().text();
    db.close();
    return false;
  };

  // Insert super_user — sesuaikan query dengan skema tabel Anda
  auto& am = AuthManager::instance();
  auto salt = am.generateSalt();
  auto hash = am.generateHash(password, salt);
  QSqlQuery q(db);
  qInfo() << "[DatabaseManager::initializeFromSetup] databaseConnection:"
          << db.connectionName();
  qInfo() << "[DatabaseManager::initializeFromSetup] databaseName:"
          << db.databaseName();
  q.prepare(
      "INSERT INTO admins (username, password_hash, nama_lengkap, salt, "
      "role_id) VALUES (:un, :pw, :nl, :st, 1)");
  q.bindValue(":un", superUser);
  q.bindValue(":pw", hash);
  q.bindValue(":nl", superUser);
  q.bindValue(":st", salt);

  if (!q.exec()) {
    qWarning()
        << "[DatabaseManager::initializeFromSetup] Super user creation failed:"
        << q.lastError().text();
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

void DatabaseManager::setDatabase(QSqlDatabase& db) {
  m_database = db;
  m_databaseReady = true;
}

DatabaseManager::DatabaseManager()
    : m_databaseReady(false), m_isFirstRun(true) {
  QSettings settings;
  auto dbPath = settings.value("Database/databasePath").toString();
  if (!dbPath.isEmpty()) {
    QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName(dbPath);
    if (_db.open()) {
      m_database = _db;
      m_databaseReady = true;
      m_isFirstRun = false;
    } else {
      m_database = QSqlDatabase();
    }
  }  // dbPath.isEmpty == true
}

bool DatabaseManager::initSchema(QSqlDatabase& db) {
  QFile sf(":/schema/main.sql");
  if (!sf.open(QIODevice::ReadOnly)) {
    qWarning() << "[DatabaseManager::initSchema] Failed to open schema file";
    return false;
  }

  QTextStream ts(&sf);
  QString s = ts.readAll();
  return false;
}

DatabaseManager::~DatabaseManager() {
  if (m_database.isValid() && m_database.isOpen()) {
    m_database.close();
  }
}

bool DatabaseManager::migrate() {
    if (!m_databaseReady) return false;
    int databaseVersion = 0;

    QSqlQuery q(m_database);

    if (m_database.tables().contains("meta")) {
        if (!q.exec("SELECT version FROM meta;")) {
            qWarning() << "[DatabaseManager::migrate] Failed to read meta:" << q.lastError().text();
            return false;
        }
        if (q.next()) {
            databaseVersion = q.value(0).toInt();
        } else {
            qWarning() << "[DatabaseManager::migrate] meta table exists but has no row";
            return false;
        }
    } else {  // no meta
        if (!q.exec("CREATE TABLE meta (version INTEGER, updated_at TEXT);")) {
            qWarning() << "[DatabaseManager::migrate] Failed to create meta table:" << q.lastError().text();
            return false;
        }

        // Hitung tabel *sebelum* meta dibuat sebetulnya lebih akurat,
        // tapi karena meta sudah terlanjur dibuat, offset +1 diperhitungkan di sini.
        bool isPreExistingDb = m_database.tables().contains("admins"); // tabel admin harus ada

        int initialVersion = isPreExistingDb ? 1 : 0;
        QSqlQuery insertQuery(m_database);
        insertQuery.prepare("INSERT INTO meta (version, updated_at) VALUES (:ver, CURRENT_TIMESTAMP);");
        insertQuery.bindValue(":ver", initialVersion);
        if (!insertQuery.exec()) {
            qWarning() << "[DatabaseManager::migrate] Failed to seed meta:" << insertQuery.lastError().text();
            return false;
        }
        databaseVersion = initialVersion;
    }

    // Lacak script skema dalam resources dengan pattern "/schema/v[1..n].sql"
    QMap<int, QString> versions;
    int version = 1;
    while (QFile::exists(QString(":/schema/schema/v%1.sql").arg(version))) {
        versions.insert(version, QString(":/schema/schema/v%1.sql").arg(version));
        ++version;
    }
    int lastVersion = version - 1;

    // Jika database sudah up-to-date
    if (databaseVersion >= lastVersion) {
        return true;
    }

    // Validasi: pastikan tidak ada gap antara databaseVersion+1 .. lastVersion
    for (int v = databaseVersion + 1; v <= lastVersion; ++v) {
        if (!versions.contains(v)) {
            qWarning() << "[DatabaseManager::migrate] Missing schema file for version" << v;
            return false;
        }
    }

    // Proses migrasi dengan transaksi
    if (!m_database.transaction()) {
        qWarning() << "[DatabaseManager::migrate] Failed to start transaction";
        return false;
    }

    while (databaseVersion < lastVersion) {
        int nextVersion = databaseVersion + 1;
        QString schemaPath = versions.value(nextVersion);

        QFile sf(schemaPath);
        if (!sf.open(QIODevice::ReadOnly)) {
            qWarning() << "[DatabaseManager::migrate] Failed to open schema file:" << schemaPath;
            m_database.rollback();
            return false;
        }

        QTextStream ts(&sf);
        QString s = ts.readAll();
        QStringList queries = s.split("---- SEP", Qt::SkipEmptyParts); // aturan pemisah sesuai konvensi skema
        for (const auto& query : queries) {
            QString trimmedQuery = query.trimmed();
            if (!trimmedQuery.isEmpty()) {
                if (!q.exec(trimmedQuery)) {
                    QString error = q.lastError().isValid() ? q.lastError().text() : "Unknown error";
                    qWarning() << QString("Failed while migrating from %1 to %2 at %3 with error: %4")
                                      .arg(databaseVersion)
                                      .arg(nextVersion)
                                      .arg(queries.indexOf(query))
                                      .arg(error);
                    m_database.rollback();
                    return false;
                }
            }
        }

        QSqlQuery updateQuery(m_database);
        updateQuery.prepare("UPDATE meta SET version = :ver, updated_at = CURRENT_TIMESTAMP;");
        updateQuery.bindValue(":ver", nextVersion);
        if (!updateQuery.exec()) {
            qWarning() << "[DatabaseManager::migrate] Failed to update meta version:" << updateQuery.lastError().text();
            m_database.rollback();
            return false;
        }

        databaseVersion = nextVersion;
    }

    if (!m_database.commit()) {
        qWarning() << "[DatabaseManager::migrate] Failed to commit transaction:" << m_database.lastError().text();
        return false;
    }
    m_isFirstRun = false;
    return true;
}

bool DatabaseManager::isOpen() const { return m_database.isOpen(); }

QSqlError DatabaseManager::lastError() const { return m_database.lastError(); }

bool DatabaseManager::verifySchema(QSqlDatabase& db) {
  // Lakukan verifikasi skema, untuk sementara di skip karena baru ada 1 skema
  return true;
}