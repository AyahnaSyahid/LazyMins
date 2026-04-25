#include "tstdatabase.h"

#include <QTest>
#include <QSqlDatabase>
#include "src/database/databasemanager.h"
#include "src/managers/managers.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/authmanager.h"
#include <QVariant>
#include <QVariantMap>

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
    QCOMPARE(aman.passwordMatch("root2", "nahalakamarada"), true);
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

void tstDatabase::tstCreateCustomer()
{
    KonsumenManager km;
    auto opt = km.create({ { "nama_lengkap", "Dadan BNG" } });
    QVERIFY2(opt.has_value(), "Gagal membuat Konsumen");
    bool updateOk = km.update(opt->value("id").toInt(), { { "nomor_telp", "08123456789" } });
    QVERIFY2(updateOk, "Gagal memperbarui Konsumen");
}

void tstDatabase::tstCreateProduct()
{
    ProductCategoryManager pcm;
    auto opt = pcm.create({ 
        { "category_name", "Pakaian" },
        { "description", "Kaos Oblong Polosan" }
     });
    QVERIFY2(opt.has_value(), "Gagal membuat Kategori Produk");
    ProductManager pm;
    auto opt2 = pm.create({ 
        { "sku", "KPATR1" },
        { "name", "Attira" },
        { "cost_price", 8000 },
        { "stock", 100 },
        { "category_id", opt->value("id").toInt() },
        { "description", "Kaos Polos Attira Wara Dongker" },
     });
    QVERIFY2(opt2.has_value(), "Gagal membuat Produk");
    bool updateOk = pm.update(opt2->value("id").toInt(), { { "cost_price", 10000 } });
    QVERIFY2(updateOk, "Gagal memperbarui Produk");
}

void tstDatabase::tstCreateOrder()
{

    KonsumenManager km;
    auto optk = km.getById(1);
    QVERIFY2(optk.has_value(), "Gagal mendapatkan Konsumen");
    auto optu = SessionManager::instance().currentUser();
    QVERIFY2(optu.has_value(), "Gagal mendapatkan User");
    OrderManager om;
    auto opt = om.create({ 
        { "customer_id", optk->value("id").toInt() },
        { "customer_name", optk->value("nama_lengkap").toString() },
        { "admin_id", optu->value("id").toInt() },
        { "created_at", QDate::currentDate() },
        { "updated_at", QDate::currentDate() }
    });
    OrderItemManager oim;
    auto opti = oim.create({
        { "order_id", opt->value("id").toInt() },
        { "product_id", 1 },
        { "product_name", "Kaos Dewasa" },
        { "quantity", 1 },
    });

    bool calcOk = om.recalculate(opt->value("id").toInt());
    QVERIFY2(calcOk, "Gagal rekalkulasi order" );

    opt = om.getById(opt->value("id").toInt());
    QVERIFY2(opt.has_value(), "Gagal mendapatkan Order");

    auto opt_subt = opt->value("subtotal").toInt();
    qDebug() << opt_subt;
    QVERIFY2(opt_subt == 10000, "Subtotal tidak valid");
}

void tstDatabase::tstCreateInvoice()
{
    InvoiceManager iman;
    OrderManager om;
    auto opto = om.getById(1);
    QVERIFY2(opto.has_value(), "Gagal mendapatkan Order");

    auto opti = iman.create({
        { "admin_id", 1 },
        { "customer_name", opto->value("customer_name") },
        { "created_at", QDate::currentDate() },
        { "updated_at", QDate::currentDate() }
    });

    QVERIFY2(opti.has_value(), "Gagal membuat Invoice");
    bool addOk = iman.addOrders(opti->value("id").toInt(), { opto->value("id").toInt() });
    QVERIFY2(addOk, "Gagal menambahkan Order ke Invoice");
    bool recalOk = iman.recalculate(opti->value("id").toInt());
    QVERIFY2(recalOk, "Gagal rekalkulasi Invoice");
    auto opt = iman.getById(opti->value("id").toInt());
    QVERIFY2(opt.has_value(), "Gagal mendapatkan Invoice");
    auto opt_subt = opt->value("subtotal").toInt();
    QVERIFY2(opt_subt == 10000, "Subtotal tidak valid");
}

void tstDatabase::tstCreatePayment()
{
}

QTEST_MAIN(tstDatabase)
