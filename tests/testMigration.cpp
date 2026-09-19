#include "testMigration.h"
#include "src/database/databasemanager.h"
#include "src/database/initializeschema.h"
#include <QDebug>
#include <QFile>
#include <QResource>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTest>
#include <QTextStream>

void TestMigration::initTestCase() {
  qApp->setApplicationName("Test");
  qApp->setOrganizationName("BlackCircle");
  Q_INIT_RESOURCE(database_resources);
}

QSqlDatabase TestMigration::openInMemoryDb(const QString &connectionName) {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
  db.setDatabaseName(":memory:");
  if (!db.open()) {
    qCritical() << "Failed to open in-memory database:"
                << db.lastError().text();
  }
  return db;
}

bool TestMigration::createV1Schema(QSqlDatabase &db) {
  QFile f(":/schema/schema/v1.sql");
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qCritical() << "Cannot open v1.sql resource";
    return false;
  }
  QTextStream ts(&f);

  QSqlQuery q(db);
  QString statement;
  bool insideCreateTrigger = false;
  int caseDepth = 0;

  while (!ts.atEnd()) {
    QString line = ts.readLine();

    if (line.trimmed().isEmpty())
      continue;
    if (line.trimmed().startsWith("--"))
      continue;

    if (line.toLower().contains("create trigger")) {
      insideCreateTrigger = true;
      caseDepth = 0;
      statement += "\n" + line;
      continue;
    }

    if (insideCreateTrigger) {
      QString stripped = line.trimmed().toLower();
      if (stripped.startsWith("case")) {
        caseDepth++;
      }
      statement += "\n" + line;
      if (stripped.contains("end;")) {
        if (caseDepth > 0) {
          caseDepth--;
        } else {
          insideCreateTrigger = false;
          if (!q.exec(statement.trimmed())) {
            qCritical() << "Trigger exec failed:" << q.lastError().text();
            f.close();
            return false;
          }
          statement.clear();
        }
      }
      continue;
    }

    if (line.contains(";")) {
      statement += "\n" + line;
      QString trimmed = statement.trimmed();
      if (!trimmed.isEmpty()) {
        QString lower = trimmed.toLower();
        if (!lower.startsWith("pragma") &&
            !lower.startsWith("begin transaction") &&
            !lower.startsWith("commit") && !lower.startsWith("---- sep")) {
          if (!q.exec(trimmed)) {
            qCritical() << "Schema exec failed:" << q.lastError().text();
            qCritical() << "Statement:" << trimmed.left(200);
            f.close();
            return false;
          }
        }
      }
      statement.clear();
      continue;
    }

    statement += "\n" + line;
  }

  if (!statement.trimmed().isEmpty()) {
    QString trimmed = statement.trimmed();
    QString lower = trimmed.toLower();
    if (!lower.startsWith("pragma") && !lower.startsWith("begin transaction") &&
        !lower.startsWith("commit")) {
      if (!q.exec(trimmed)) {
        qCritical() << "Schema exec failed (flush):" << q.lastError().text();
        f.close();
        return false;
      }
    }
  }

  f.close();
  return true;
}

bool TestMigration::setMetaVersion(QSqlDatabase &db, int version) {
  QSqlQuery q(db);
  q.prepare("INSERT OR REPLACE INTO meta (version, updated_at) VALUES (:ver, "
            "CURRENT_TIMESTAMP)");
  q.bindValue(":ver", version);
  return q.exec();
}

bool TestMigration::hasAllV1Tables(QSqlDatabase &db) {
  QStringList expectedTables = {"roles",
                                "admins",
                                "price_levels",
                                "product_categories",
                                "products",
                                "product_prices",
                                "finishing_services",
                                "konsumen",
                                "app_settings",
                                "meta",
                                "akun_transaksi",
                                "kategori_transaksi",
                                "orders",
                                "order_items",
                                "order_item_finishings",
                                "invoices",
                                "payments",
                                "transaksi",
                                "stock_movements",
                                "activity_logs"};
  QStringList actualTables = db.tables();
  for (const QString &table : expectedTables) {
    if (!actualTables.contains(table))
      return false;
  }
  return true;
}

void TestMigration::testNewDatabaseEmpty() {
  QSqlDatabase db = openInMemoryDb("test_new_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  QVERIFY(!db.tables().contains("meta"));
  QVERIFY(!db.tables().contains("admins"));

  bool result = dbm.migrate();
  QVERIFY2(result, "migrate() should succeed on new DB");
  QVERIFY(hasAllV1Tables(db));
  QVERIFY(dbm.isFirstRun() == false);

  QSqlQuery q(db);
  QVERIFY(q.exec("SELECT version FROM meta"));
  QVERIFY(q.next());
  QCOMPARE(q.value(0).toInt(), 1);
}

void TestMigration::testAlreadyCurrent() {
  QSqlDatabase db = openInMemoryDb("test_current_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  QVERIFY(createV1Schema(db));
  QVERIFY(setMetaVersion(db, 1));

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  bool result = dbm.migrate();
  QVERIFY2(result, "migrate() should succeed on current DB");
  QVERIFY(hasAllV1Tables(db));

  QSqlQuery q(db);
  QVERIFY(q.exec("SELECT version FROM meta"));
  QVERIFY(q.next());
  QCOMPARE(q.value(0).toInt(), 1);
}

void TestMigration::testOutOfDateDatabase() {
  QSqlDatabase db = openInMemoryDb("test_outdated_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  QSqlQuery q(db);
  QVERIFY(q.exec("CREATE TABLE meta (version INTEGER, updated_at TEXT)"));
  QVERIFY(q.exec(
      "INSERT INTO meta (version, updated_at) VALUES (0, CURRENT_TIMESTAMP)"));

  QVERIFY(db.tables().contains("meta"));
  QVERIFY(!db.tables().contains("admins"));
  QVERIFY(!db.tables().contains("roles"));

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  bool result = dbm.migrate();
  QVERIFY2(result, "migrate() should succeed on out-of-date DB");
  QVERIFY(hasAllV1Tables(db));

  QSqlQuery q2(db);
  QVERIFY(q2.exec("SELECT version FROM meta"));
  QVERIFY(q2.next());
  QCOMPARE(q2.value(0).toInt(), 1);
}

void TestMigration::testErrorRollback() {
  QSqlDatabase db = openInMemoryDb("test_rollback_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  QVERIFY(createV1Schema(db));
  QVERIFY(setMetaVersion(db, 0));

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  bool result = dbm.migrate();
  QVERIFY2(!result,
           "migrate() should fail when tables already exist and version is 0");

  QVERIFY(hasAllV1Tables(db));

  QSqlQuery q(db);
  QVERIFY(q.exec("SELECT version FROM meta"));
  QVERIFY(q.next());
  QCOMPARE(q.value(0).toInt(), 0);
}

void TestMigration::testPreExistingDbWithAdmins() {
  QSqlDatabase db = openInMemoryDb("test_preexisting_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  QSqlQuery q(db);
  QVERIFY(q.exec("CREATE TABLE admins ("
                 "  id INTEGER PRIMARY KEY,"
                 "  role_id INTEGER NOT NULL DEFAULT 3,"
                 "  username TEXT NOT NULL UNIQUE,"
                 "  password_hash TEXT NOT NULL,"
                 "  salt TEXT NOT NULL,"
                 "  nama_lengkap TEXT NOT NULL,"
                 "  email TEXT,"
                 "  nomor_telp TEXT,"
                 "  is_active INTEGER DEFAULT 1,"
                 "  last_login DATETIME,"
                 "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                 "  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                 ")"));
  QVERIFY(q.exec(
      "INSERT INTO admins (id, username, password_hash, salt, nama_lengkap) "
      "VALUES (1, 'admin', 'hash', 'salt', 'Admin User')"));

  QStringList tables = db.tables();
  QVERIFY(tables.contains("admins"));
  QVERIFY(!tables.contains("meta"));
  QVERIFY(!tables.contains("roles"));
  QVERIFY(!tables.contains("products"));

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  bool result = dbm.migrate();
  QVERIFY2(result, "migrate() should succeed on pre-existing DB");

  QVERIFY(db.tables().contains("meta"));
  QSqlQuery mq(db);
  QVERIFY(mq.exec("SELECT version FROM meta"));
  QVERIFY(mq.next());
  QCOMPARE(mq.value(0).toInt(), 1);

  tables = db.tables();
  QVERIFY(tables.contains("admins"));
  QVERIFY(tables.contains("meta"));
  QVERIFY(!tables.contains("roles"));
  QVERIFY(!tables.contains("products"));
  QVERIFY(!tables.contains("orders"));
  QVERIFY(!tables.contains("konsumen"));
}

void TestMigration::testVersionGreaterThanLast() {
  QSqlDatabase db = openInMemoryDb("test_version_overestimate_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  QSqlQuery q(db);
  QVERIFY(q.exec("CREATE TABLE meta (version INTEGER, updated_at TEXT)"));
  QVERIFY(q.exec(
      "INSERT INTO meta (version, updated_at) VALUES (5, CURRENT_TIMESTAMP)"));

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  bool result = dbm.migrate();
  QVERIFY2(result,
           "migrate() silently accepts version > lastVersion (known gap)");

  QSqlQuery mq(db);
  QVERIFY(mq.exec("SELECT version FROM meta"));
  QVERIFY(mq.next());
  QCOMPARE(mq.value(0).toInt(), 5);
}

void TestMigration::testMetaTableStructure() {
  QSqlDatabase db = openInMemoryDb("test_meta_structure_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  bool result = dbm.migrate();
  QVERIFY(result);

  QSqlQuery pq(db);
  QVERIFY(pq.exec("PRAGMA table_info(meta)"));
  int colCount = 0;
  QString versionName, versionType, updatedName, updatedType;
  while (pq.next()) {
    ++colCount;
    QString cname = pq.value("name").toString();
    QString ctype = pq.value("type").toString();
    if (cname == "version") {
      versionName = cname;
      versionType = ctype;
    } else if (cname == "updated_at") {
      updatedName = cname;
      updatedType = ctype;
    }
  }
  QCOMPARE(colCount, 2);
  QCOMPARE(versionName, QString("version"));
  QCOMPARE(versionType, QString("INTEGER"));
  QCOMPARE(updatedName, QString("updated_at"));
  QCOMPARE(updatedType, QString("TEXT"));
}

void TestMigration::testAllTablesExistAfterMigration() {
  QSqlDatabase db = openInMemoryDb("test_all_tables_db");
  QVERIFY2(db.isOpen(), "Failed to open in-memory DB");

  DatabaseManager &dbm = DatabaseManager::instance();
  dbm.setDatabase(db);

  bool result = dbm.migrate();
  QVERIFY(result);

  QStringList expected = {"activity_logs",
                          "admins",
                          "akun_transaksi",
                          "app_settings",
                          "finishing_services",
                          "invoices",
                          "kategori_transaksi",
                          "konsumen",
                          "meta",
                          "order_item_finishings",
                          "order_items",
                          "orders",
                          "payments",
                          "price_levels",
                          "product_categories",
                          "product_prices",
                          "products",
                          "roles",
                          "stock_movements",
                          "transaksi"};

  QStringList actual = db.tables();
  for (const QString &t : expected) {
    QVERIFY2(actual.contains(t), QString("Table '%1' missing after migration. "
                                         "Available: %2")
                                     .arg(t)
                                     .arg(actual.join(", "))
                                     .toUtf8()
                                     .constData());
  }
}

void TestMigration::init()
{
    DatabaseManager &dbm = DatabaseManager::instance();
    if (dbm.database().isValid() && dbm.database().isOpen()) {
        dbm.database().close();
    }
}

void TestMigration::cleanupTestCase()
{
    // Release DatabaseManager's hold on the last connection so
    // removeDatabase doesn't complain about "still in use".
    DatabaseManager &dbm = DatabaseManager::instance();
    if (dbm.database().isValid() && dbm.database().isOpen()) {
        dbm.database().close();
    }
    static QSqlDatabase nullDb;
    dbm.setDatabase(nullDb);

    QStringList connections = QSqlDatabase::connectionNames();
    for (const QString &conn : connections) {
        QSqlDatabase::removeDatabase(conn);
    }
}

QTEST_MAIN(TestMigration)
#include "testMigration.moc"