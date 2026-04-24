#include "tstdatabase.h"

#include <QTest>
#include <QSqlDatabase>
#include "src/database/databasemanager.h"
#include "src/managers/managers.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/authmanager.h"

void tstDatabase::initTestCase()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE", "lazyConnection");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        QFAIL("Database not open");
    }
    auto &dm = DatabaseManager::instance();
    if ( !DatabaseManager::initSchema(db) ) {
        QFAIL("Database schema initialization failed");
    }
    BaseManager::connection = db;
}

void tstDatabase::tstCreateRootUser() {
    AdminManager aman;
    AuthManager &aum = AuthManager::instance();
    auto opt = aman.create({
        { "username",         "root2"},
        { "role_id",          1 },
        { "literal_password", "nahalakamarada" },
        { "nama_lengkap",     "NurHolis K" },
        { "email",            "email" },
        { "nomor_telp",       "phone" },
        { "is_active",        true }
    });

    QCOMPARE(opt.has_value(), true);
    QCOMPARE(aum.passwordMatch("root2", "nahalakamarada"), true);
}

void tstDatabase::tstLoginWithRootUser()
{
    auto &sm = SessionManager::instance();
    auto opt = sm.currentUser();
    QCOMPARE(opt.has_value(), false);
    sm.login("root2", "nahalakamarada");
    opt = sm.currentUser();
    QCOMPARE(opt.has_value(), true);
    QCOMPARE(opt->value("username").toString(), "root2");
}

void tstDatabase::tstCreateKasirUser()
{
    AdminManager aman;
    auto opt = aman.create({
        { "username",         "kasir2"},
        { "role_id",          2 },
        { "literal_password", "nahamanehkitu" },
        { "nama_lengkap",     "Kasir Gelo" },
        { "email",            "email" },
        { "nomor_telp",       "phone" },
        { "is_active",        true }
    });

    QCOMPARE(opt.has_value(), true);
}

void tstDatabase::tstChangeCurrentUser()
{
    auto &sm = SessionManager::instance();
    sm.login("root2", "nahalakamarada");
    auto opt = sm.currentUser();
    QCOMPARE(opt.has_value(), true);
    QCOMPARE(opt->value("username").toString(), "root2");
    sm.login("kasir2", "nahamanehkitu");
    opt = sm.currentUser();
    QCOMPARE(opt.has_value(), true);
    QCOMPARE(opt->value("username").toString(), "kasir2");
}

QTEST_MAIN(tstDatabase)
