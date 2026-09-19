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

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager dbm;
    return dbm;
}

bool DatabaseManager::initializeFromSetup(const QString &dbPath,
                                          const QString &superUser,
                                          const QString &password)
{
    QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE", "setup");
    _db.setDatabaseName(dbPath);
    if (!_db.open())
    {
        qCritical() << "Database Open Failed on setup";
        return false;
    }
    DatabaseManager &dbm = DatabaseManager::instance();
    dbm.setDatabase(_db);
    if (!dbm.migrate())
    {
        return false;
    }

    AuthManager &am = AuthManager::instance();
    auto salt = am.generateSalt();
    auto hash = am.generateHash(password, salt);

    QSqlQuery q(_db);
    q.prepare("INSERT INTO admins (username, password_hash, salt, nama_lengkap) VALUES (:un, :ph, :sl, :nl)");
    q.bindValue(":un", superUser);
    q.bindValue(":ph", hash);
    q.bindValue(":sl", salt);
    q.bindValue(":nl", "SuperUser01");
    if (!q.exec())
    {
        QString errMsg("Failed to initialize root user");
        if (q.lastError().isValid())
        {
            errMsg += " " + q.lastError().text();
        }
        _db.close();
        // need to remove unsucessfull database creation here
        return false;
    }
    m_isFirstRun = false;
    QSettings s;
    // below line is for test purpose where settings maybe uninitialized
    if (s.status() != QSettings::NoError)
        return true;
    s.setValue(Config::Database::SETTINGS_KEY_DBPATH, dbPath);
    s.sync();
    return true;
}

void DatabaseManager::setDatabase(QSqlDatabase &db)
{
    m_database = db;
    m_databaseReady = true;
}

DatabaseManager::DatabaseManager()
    : m_databaseReady(false), m_isFirstRun(true)
{
    QSettings settings;
    auto dbPath = settings.value(Config::Database::SETTINGS_KEY_DBPATH, "").toString();
    if (!dbPath.isEmpty())
    {
        QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE");
        _db.setDatabaseName(dbPath);
        if (_db.open())
        {
            m_database = _db;
            m_databaseReady = true;
            m_isFirstRun = false;
        }
        else
        {
            m_database = QSqlDatabase();
        }
    } // dbPath.isEmpty == true
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isValid() && m_database.isOpen())
    {
        m_database.close();
    }
}

bool DatabaseManager::migrate()
{
    if (!m_databaseReady)
        return false;
    int databaseVersion = 0;

    QSqlQuery q(m_database);

    if (m_database.tables().contains("meta"))
    {
        if (!q.exec("SELECT version FROM meta;"))
        {
            qWarning() << "[DatabaseManager::migrate] Failed to read meta:" << q.lastError().text();
            return false;
        }
        if (q.next())
        {
            databaseVersion = q.value(0).toInt();
        }
        else
        {
            qWarning() << "[DatabaseManager::migrate] meta table exists but has no row";
            return false;
        }
    }
    else
    { // no meta
        if (!q.exec("CREATE TABLE meta (version INTEGER, updated_at TEXT);"))
        {
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
        if (!insertQuery.exec())
        {
            qWarning() << "[DatabaseManager::migrate] Failed to seed meta:" << insertQuery.lastError().text();
            return false;
        }
        databaseVersion = initialVersion;
    }

    // Lacak script skema dalam resources dengan pattern "/schema/v[1..n].sql"
    QMap<int, QString> versions;
    int version = 1;
    while (QFile::exists(QString(":/schema/schema/v%1.sql").arg(version)))
    {
        versions.insert(version, QString(":/schema/schema/v%1.sql").arg(version));
        ++version;
    }
    int lastVersion = version - 1;

    // Jika database sudah up-to-date
    if (databaseVersion >= lastVersion)
    {
        return true;
    }

    // Validasi: pastikan tidak ada gap antara databaseVersion+1 .. lastVersion
    for (int v = databaseVersion + 1; v <= lastVersion; ++v)
    {
        if (!versions.contains(v))
        {
            qWarning() << "[DatabaseManager::migrate] Missing schema file for version" << v;
            return false;
        }
    }

    // Proses migrasi dengan transaksi
    if (!m_database.transaction())
    {
        qWarning() << "[DatabaseManager::migrate] Failed to start transaction";
        return false;
    }

    while (databaseVersion < lastVersion)
    {
        int nextVersion = databaseVersion + 1;
        QString schemaPath = versions.value(nextVersion);

        QFile sf(schemaPath);
        if (!sf.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << "[DatabaseManager::migrate] Failed to open schema file:" << schemaPath;
            m_database.rollback();
            return false;
        }

        // Parse schema file line-by-line, accumulating statements.
        // This handles multi-statement chunks (e.g. PRAGMA + BEGIN + CREATE TABLE)
        // that the ---- SEP split cannot handle.
        QTextStream ts(&sf);
        QString statement;
        bool insideCreateTrigger = false;
        int caseDepth = 0;

        while (!ts.atEnd())
        {
            QString line = ts.readLine();

            if (line.trimmed().isEmpty()) continue;
            if (line.trimmed().startsWith("--")) continue;

            if (line.toLower().contains("create trigger"))
            {
                insideCreateTrigger = true;
                caseDepth = 0;
                statement += "\n" + line;
                continue;
            }

            if (insideCreateTrigger)
            {
                QString stripped = line.trimmed().toLower();
                if (stripped.startsWith("case")) caseDepth++;
                statement += "\n" + line;
                if (stripped.contains("end;"))
                {
                    if (caseDepth > 0) { caseDepth--; }
                    else
                    {
                        insideCreateTrigger = false;
                        if (!q.exec(statement.trimmed()))
                        {
                            QString error = q.lastError().isValid() ? q.lastError().text() : "Unknown error";
                            qWarning() << QString("[DatabaseManager::migrate] Failed migrating %1->%2 trigger: %3")
                                          .arg(databaseVersion).arg(nextVersion).arg(error);
                            m_database.rollback();
                            return false;
                        }
                        statement.clear();
                    }
                }
                continue;
            }

            if (line.contains(";"))
            {
                statement += "\n" + line;
                QString trimmed = statement.trimmed();
                if (!trimmed.isEmpty())
                {
                    // Skip transaction control statements — migrate() manages
                    // the outer transaction wrapper.
                    QString lower = trimmed.toLower();
                    if (lower.startsWith("begin transaction") ||
                        lower.startsWith("commit"))
                    {
                        statement.clear();
                        continue;
                    }
                    // Skip CREATE TABLE meta — migrate() handles meta table itself
                    if (lower.startsWith("create table meta"))
                    {
                        statement.clear();
                        continue;
                    }
                    if (!q.exec(trimmed))
                    {
                        QString error = q.lastError().isValid() ? q.lastError().text() : "Unknown error";
                        qWarning() << QString("[DatabaseManager::migrate] Failed migrating %1->%2: %3")
                                      .arg(databaseVersion).arg(nextVersion).arg(error);
                        m_database.rollback();
                        return false;
                    }
                }
                statement.clear();
                continue;
            }

            statement += "\n" + line;
        }

        // Flush any remaining statement
        if (!statement.trimmed().isEmpty())
        {
            if (!q.exec(statement.trimmed()))
            {
                QString error = q.lastError().isValid() ? q.lastError().text() : "Unknown error";
                qWarning() << QString("[DatabaseManager::migrate] Failed migrating %1->%2 (flush): %3")
                              .arg(databaseVersion).arg(nextVersion).arg(error);
                m_database.rollback();
                return false;
            }
        }

        sf.close();

        QSqlQuery updateQuery(m_database);
        updateQuery.prepare("UPDATE meta SET version = :ver, updated_at = CURRENT_TIMESTAMP;");
        updateQuery.bindValue(":ver", nextVersion);
        if (!updateQuery.exec())
        {
            qWarning() << "[DatabaseManager::migrate] Failed to update meta version:" << updateQuery.lastError().text();
            m_database.rollback();
            return false;
        }

        databaseVersion = nextVersion;
    }

    if (!m_database.commit())
    {
        qWarning() << "[DatabaseManager::migrate] Failed to commit transaction:" << m_database.lastError().text();
        return false;
    }
    m_isFirstRun = false;
    return true;
}

QSqlError DatabaseManager::lastError() const { return m_database.lastError(); }

bool DatabaseManager::verifySchema(QSqlDatabase &db)
{
    // Lakukan verifikasi skema, untuk sementara di skip karena baru ada 1 skema
    return true;
}