#include "tstdatabase.h"

#include <QTest>
#include <QSqlDatabase>
#include "src/database/databasemanager.h"
#include "src/managers/managers.h"
#include "src/managers/financialledgerservice.h"
#include "src/managers/itemflowservice.h"
#include "src/utils/sessionmanager.h"
#include "src/utils/sqltransaction.h"
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
    QSqlQuery q(db);
    q.exec("PRAGMA foreign_keys = ON;");
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

    auto accl = aman.getAll();
    QCOMPARE(accl.count(), 1);
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

    auto custs = km.getAll();
    QCOMPARE(custs.count() > 1, true);
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

void tstDatabase::tstItemFlowMechanism()
{
    ProductManager pm;
    OrderItemManager oim;
    OrderManager om;
    ItemFlowService its;
    int adminId = SessionManager::instance().currentUserId();

    // ---------------------------------------------------------
    // 1. SETUP: Ambil Produk untuk Testing (Misal: Produk Satuan)
    // ---------------------------------------------------------
    auto optp1 = pm.getBySku("KPATR1"); 
    QVERIFY2(optp1.has_value(), "Gagal: Produk KPATR1 tidak ditemukan di database");
    double stockAwal = optp1->value("stock").toDouble();
    int prodId1 = optp1->value("id").toInt();

    // ---------------------------------------------------------
    // 2. SETUP: Buat Order
    // ---------------------------------------------------------
    auto opto = om.create({ 
        { "customer_name", "Dadan BNG" },
        { "created_at", QDate::currentDate() },
        { "updated_at", QDate::currentDate() },
        { "admin_id", adminId }
    });
    QVERIFY2(opto.has_value(), "Gagal membuat Order");

    // ---------------------------------------------------------
    // 3. TEST: Penjualan Normal (Quantity Based)
    // ---------------------------------------------------------

    qDebug() << "Product Name:" << optp1->value("name").toString();

    int qtyJual = 10;
    auto optItem1 = oim.create({
        { "order_id", opto->value("id") },
        { "product_id", prodId1 },
        { "product_name", optp1->value("name")},
        { "quantity", qtyJual },
        { "sale_price", optp1->value("cost_price") },
        { "base_price", optp1->value("cost_price") },
        { "use_area", false }
    });
    QVERIFY(optItem1.has_value());

    QVERIFY2(its.handleItemSold(optItem1->value("id").toInt()), qPrintable(its.errorString()));
    
    // Verifikasi Stok
    auto pAfter1 = pm.getById(prodId1);
    QCOMPARE(pAfter1->value("stock").toDouble(), stockAwal - qtyJual);

    // ---------------------------------------------------------
    // 3. TEST: Penjualan Produk Area (Printing/Meteran)
    // ---------------------------------------------------------
    // Misal: Banner 1.2m x 1.0m sebanyak 2 pcs = 2.4m
    double w = 1.2, h = 1.0, q = 2.0;
    double expectedUsage = qCeil((w * h * q) * 100.0) / 100.0; // 2.4

    auto pr_area = pm.getWhere("use_area = :cond", {{"cond", 1}}, "", 1);
    
    if (!pr_area.count())
        QFAIL("Produk Area tidak ditemukan");

    auto prodArea = pr_area.first();
    double stockBeforeArea = prodArea.value("stock").toDouble();
    auto optItemArea = oim.create({
        { "order_id", opto->value("id") },
        { "product_id", prodArea.value("id") },
        { "product_name", "CUSTOM 01" },
        { "quantity", q },
        { "use_area", true },
        { "sale_price",  prodArea.value("cost_price").toInt() },
        { "base_price",  prodArea.value("cost_price").toInt() },
        { "size_width", w },
        { "size_height", h }
    });

    QVERIFY2(its.handleItemSold(optItemArea->value("id").toInt()), qPrintable(its.errorString()));
    
    // Verifikasi Stok (Harus berkurang sesuai luas, bukan sekadar quantity)
    auto pAfterArea = pm.getById(prodArea.value("id").toInt());
    QCOMPARE(pAfterArea->value("stock").toDouble(), stockBeforeArea - expectedUsage);

    // ... (Bagian 1-3 sudah benar)

    // ---------------------------------------------------------
    // 4. TEST: Pembatalan & Restock (Kembali ke Stok)
    // ---------------------------------------------------------
    // Ambil stok terbaru produk satuan sebelum dibatalkan
    double stockSatuanBeforeCancel = pm.getById(prodId1)->value("stock").toDouble();

    // Batalkan item pertama (Produk Satuan: qty 10) dengan restock = true
    QVERIFY2(its.handleItemCanceled(optItem1->value("id").toInt(), true), qPrintable(its.errorString()));

    // VERIFIKASI: Harus dibandingkan dengan prodId1, bukan prodArea
    auto pAfterRestockSatuan = pm.getById(prodId1);
    QCOMPARE(pAfterRestockSatuan->value("stock").toDouble(), stockSatuanBeforeCancel + qtyJual);


    // ---------------------------------------------------------
    // 5. TEST: Pembatalan TANPA Restock (Barang Rusak)
    // ---------------------------------------------------------
    // Ambil stok terbaru produk area sebelum dibatalkan
    double stockAreaBeforeCancel = pm.getById(prodArea.value("id").toInt())->value("stock").toDouble();

    // Batalkan item area dengan restock = false
    QVERIFY2(its.handleItemCanceled(optItemArea->value("id").toInt(), false), qPrintable(its.errorString()));

    // VERIFIKASI: Stok area tidak boleh berubah karena restock = false
    auto pFinalArea = pm.getById(prodArea.value("id").toInt());
    QCOMPARE(pFinalArea->value("stock").toDouble(), stockAreaBeforeCancel);

    // ---------------------------------------------------------
    // 6. TEST: Stok Minus (Sesuai kebijakan: Diizinkan)
    // ---------------------------------------------------------
    // Jual dalam jumlah sangat besar melebihi stok yang ada
    auto optItemMinus = oim.create({
        { "order_id", opto->value("id") },
        { "product_id", prodId1 },
        { "product_name", "CUSTOM 01" },
        { "quantity", 999 },
        { "use_area", false},
        { "sale_price",  prodArea.value("cost_price").toInt() },
        { "base_price",  prodArea.value("cost_price").toInt() },
        { "size_width", 1 },
        { "size_height", 1 }
    });
    
    QVERIFY2(its.handleItemSold(optItemMinus->value("id").toInt()), "Harusnya tetap bisa jual meski stok minus");
    
    auto pMinus = pm.getById(prodId1);
    QVERIFY2(pMinus->value("stock").toDouble() < 0, "Stok seharusnya bernilai negatif sekarang");
    
    om.update(opto->value("id").toInt(), {{"staging_status", "cancelled"}});

    // Simulasi panggil handleItemCanceled untuk kedua kalinya
    // Harusnya tetap return true (karena handleItemLog return true saat affected 0)
    QVERIFY2(its.handleItemCanceled(optItemArea->value("id").toInt(), true), "Harusnya silent success");

    // Pastikan stok TIDAK berubah lagi (tetap sama dengan pFinalArea)
    auto pDoubleCheck = pm.getById(prodArea.value("id").toInt());
    QCOMPARE(pDoubleCheck->value("stock").toDouble(), pFinalArea->value("stock").toDouble());
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
    
    // --- Pengujian Rekalkulasi Pertama ---
    bool recalOk = iman.recalculate(opti->value("id").toInt());
    QVERIFY2(recalOk, "Gagal rekalkulasi Invoice pertama");
    
    auto opti1 = iman.getById(opti->value("id").toInt());
    int totalAwal = opti1->value("total_amount").toInt();

    // --- Pengujian Rekalkulasi Kedua (Harusnya Nilai Tetap Sama) ---
    bool recalOk2 = iman.recalculate(opti->value("id").toInt());
    QVERIFY2(recalOk2, "Gagal rekalkulasi Invoice kedua");
    
    auto opti2 = iman.getById(opti->value("id").toInt());
    int totalAkhir = opti2->value("total_amount").toInt();

    // VERIFIKASI: Nilai tidak boleh berubah/bertambah ganda
    QCOMPARE(totalAkhir, totalAwal);
    
    // Bandingkan dengan subtotal order
    opto = om.getById(opto->value("id").toInt());
    QCOMPARE(totalAkhir, opto->value("subtotal").toInt());
}

void tstDatabase::tstCreateTransactionAccount()
{
    AkunTransaksiManager atman;
}

void tstDatabase::tstPaymentMechanism()
{
    InvoiceManager iman;
    PaymentManager pman;
    AkunTransaksiManager atman;
    FinancialLedgerService flc;
    int adminId = SessionManager::instance().currentUserId();

    // --- PREPARASI DATA ---
    auto opti = iman.getById(1);
    QVERIFY2(opti.has_value(), "Persiapan gagal: Invoice ID 1 tidak ditemukan");
    
    auto opta = atman.getById(1);
    QVERIFY2(opta.has_value(), "Persiapan gagal: Akun Transaksi ID 1 tidak ditemukan");
    
    int saldoAwal = opta->value("saldo").toInt();
    int tagihan = opti->value("total_amount").toInt();

    // ---------------------------------------------------------
    // 1. TEST OVERPAID (HARUS GAGAL)
    // ---------------------------------------------------------
    {
        auto optOver = pman.create({
            { "invoice_id", opti->value("id").toInt() },
            { "amount", tagihan + 50000 }, // Melebihi total tagihan
            { "akun_transaksi_id", 1 },
            { "verification_status", "verified" },
            { "admin_id", adminId }
        });

        SqlTransaction tr;
        if (!optOver.has_value()) {
            QFAIL(qPrintable("Persiapan gagal: Overpaid gagal dibuat"));
        }

        bool success = flc.handlePayment(optOver->value("id").toInt());
        QVERIFY2(!success, "Logika GAGAL: Sistem seharusnya menolak Overpaid!");
        // Rollback agar data sampah overpaid tidak merusak test selanjutnya

    }

    // ---------------------------------------------------------
    // 2. TEST NORMAL PAYMENT (HARUS BERHASIL)
    // ---------------------------------------------------------
    QVariantMap payData = {
        { "invoice_id", opti->value("id").toInt() },
        { "amount", tagihan }, // Bayar lunas
        { "akun_transaksi_id", 1 },
        { "verification_status", "verified" },
        { "admin_id", adminId },
        { "verified_by", adminId },
        { "verified_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss") },
        { "created_at", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss") }
    };

    auto optPay = pman.create(payData);
    QVERIFY(optPay.has_value());

    {
        SqlTransaction tr;
        if (!flc.handlePayment(optPay->value("id").toInt())) {
            QFAIL(qPrintable("Harusnya berhasil tapi error: " + flc.errorString()));
        }
        QVERIFY(tr.commit());
    }

    // Verifikasi saldo bertambah
    auto optaAfter = atman.getById(1);
    QCOMPARE(optaAfter->value("saldo").toInt(), saldoAwal + tagihan);

    // ---------------------------------------------------------
    // 3. TEST CANCEL PAYMENT (HARUS BERHASIL)
    // ---------------------------------------------------------
    {
        SqlTransaction tr;
        // Tandai status di tabel payments jadi cancelled
        QVERIFY(pman.cancel(optPay->value("id").toInt(), adminId));

        // Jalankan ledger untuk membalik saldo
        if (!flc.handlePayment(optPay->value("id").toInt())) {
            QFAIL(qPrintable("Gagal cancel ledger: " + flc.errorString()));
        }
        QVERIFY(tr.commit());
    }

    // Verifikasi saldo kembali ke awal
    auto optaFinal = atman.getById(1);
    QCOMPARE(optaFinal->value("saldo").toInt(), saldoAwal);

    // ---------------------------------------------------------
    // 4. TEST DOUBLE CANCEL (HARUS GAGAL)
    // ---------------------------------------------------------
    // Mencoba membatalkan lagi payment yang sudah cancelled
    bool secondCancel = pman.cancel(optPay->value("id").toInt(), adminId);
    QVERIFY2(!secondCancel, "Logika GAGAL: Payment yang sudah cancel tidak boleh di-cancel lagi!");
}

QTEST_MAIN(tstDatabase)
