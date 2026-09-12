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
}

void TestDatabase::testDatabaseMigrate() {
    Q_INIT_RESOURCE(database_resources);
    DatabaseManager &dbm = DatabaseManager::instance();
    QVERIFY(QFileInfo::exists(":/schema/schema/v1.sql"));
    QVERIFY(dbm.isFirstRun() == true);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    QVERIFY(db.open() == true);
    dbm.setDatabase(db);
    QVERIFY(dbm.isFirstRun());
    QVERIFY(dbm.migrate());
    QVERIFY(dbm.isFirstRun());
    qDebug() << db.tables().count();
    QVERIFY(db.tables().count() == 19);
}

QTEST_MAIN(TestDatabase)