#include "testDatabase.h"
#include "src/database/databasemanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QFileInfo>

void TestDatabase::initTestCase() {
    qApp->setApplicationName("Test");
    qApp->setOrganizationName("BlackCircle");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    Q_INIT_RESOURCE(database_resources);
}

void TestDatabase::testDatabaseMigrate() {
    DatabaseManager &dbm = DatabaseManager::instance();
    QVERIFY(QFileInfo::exists(":/schema/schema/v1.sql"));
    QVERIFY(dbm.isFirstRun());
    auto setupFuncOk = dbm.initializeFromSetup(":memory:", "root", "000000");
    QVERIFY(setupFuncOk);
    QVERIFY(!dbm.isFirstRun());
}

void TestDatabase::testDatabaseMigrateAlready() {
    DatabaseManager &dbm = DatabaseManager::instance();
    auto db = QSqlDatabase::addDatabase("QSQLITE", "second");
    db.setDatabaseName("/mnt/external/LinuxData/Development/Project/LazyMins/LAdmins.db");
    QVERIFY(db.open());
    dbm.setDatabase(db);
    QVERIFY(dbm.migrate());
}

QTEST_MAIN(TestDatabase)