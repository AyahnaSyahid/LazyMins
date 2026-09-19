#pragma once

#include <QObject>
#include <QSqlDatabase>

class TestMigration : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();

  // Happy path: fresh DB with no meta table at all
  void testNewDatabaseEmpty();

  // Pre-existing DB: admins table exists, no meta → version seeded at 1
  void testPreExistingDbWithAdmins();

  // DB already at latest version → migrate() is a no-op
  void testAlreadyCurrent();

  // DB at version 0 → v1.sql migration runs and brings it to 1
  void testOutOfDateDatabase();

  // Bad SQL in migration → transaction rolls back, meta.version unchanged
  void testErrorRollback();

  // databaseVersion > lastVersion → silently accepted (gap in analysis)
  void testVersionGreaterThanLast();

  // Verify meta table has correct columns after migration
  void testMetaTableStructure();

  // Verify all expected v1.sql tables exist after migration
  void testAllTablesExistAfterMigration();

private:
  // Helper: create an in-memory DB with the full v1 schema applied.
  bool createV1Schema(QSqlDatabase &db);

  // Helper: create meta table with given version.
  bool setMetaVersion(QSqlDatabase &db, int version);

  // Helper: check if all expected v1 tables exist.
  bool hasAllV1Tables(QSqlDatabase &db);

  // Helper: get a fresh in-memory DB connection.
  QSqlDatabase openInMemoryDb(const QString &connectionName);
};
