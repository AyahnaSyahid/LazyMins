#pragma once

#include <cstddef>  // size_t

/* ===========================
    # description here
   =========================== */

namespace SQLITE_SCHEMA {

constexpr char *FK_OFF = "PRAGMA foreign_keys = OFF;";
constexpr char *FK_ON = "PRAGMA foreign_keys = ON;";
constexpr char *TR_BEGIN = "BEGIN TRANSACTION;";
constexpr char *COMMIT = "COMMIT;";
constexpr char *ROLLBACK = "ROLLBACK;";

// MIGRATE V1 //
namespace V1 {
constexpr char* ct_roles = R"(
    CREATE TABLE roles (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        role_name TEXT NOT NULL UNIQUE,  -- Nama role: super_admin, kasir, operator
        description TEXT,                 -- Deskripsi role
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)";

constexpr char* ct_admins = R"(
    CREATE TABLE admins (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        role_id INTEGER NOT NULL,
        username TEXT NOT NULL UNIQUE,
        password_hash TEXT NOT NULL,      -- Password ter-hash (bcrypt/argon2)
        nama_lengkap TEXT NOT NULL,
        email TEXT,                       -- TAMBAHAN: Email admin
        nomor_telp TEXT,                  -- TAMBAHAN: Nomor telepon admin
        is_active INTEGER DEFAULT 1,      -- 1: Aktif, 0: Non-aktif
        last_login DATETIME,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE RESTRICT
        );
)";
constexpr char* idx_admins_username = "CREATE INDEX idx_admins_username ON admins(username);";
constexpr char* idx_admins_role = "CREATE INDEX idx_admins_role ON admins(role_id);";
constexpr char* ct_konsumen = R"(
    CREATE TABLE konsumen (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        customer_code TEXT UNIQUE,        -- TAMBAHAN: Kode unik pelanggan (CUST-001)
        nama_lengkap TEXT NOT NULL,
        customer_type TEXT DEFAULT 'individual',  -- TAMBAHAN: individual/company
        email TEXT,                       -- Removed UNIQUE constraint (beberapa pelanggan bisa tidak punya email)
        nomor_telp TEXT,
        alamat TEXT,
        kota TEXT,                        -- TAMBAHAN: Kota pelanggan
        kode_pos TEXT,                    -- TAMBAHAN: Kode pos
        npwp TEXT,                        -- TAMBAHAN: NPWP untuk pelanggan korporat
        catatan TEXT,                     -- TAMBAHAN: Catatan khusus pelanggan
        price_level_id INTEGER DEFAULT 1, -- TAMBAHAN: Level harga default pelanggan
        is_active INTEGER DEFAULT 1,      -- TAMBAHAN: Status aktif/non-aktif
        total_orders INTEGER DEFAULT 0,   -- TAMBAHAN: Total order yang pernah dibuat
        total_spent REAL DEFAULT 0,       -- TAMBAHAN: Total pembelian
        last_seen DATETIME,               -- Terakhir kali order
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (price_level_id) REFERENCES price_levels(id)
    );
)";

constexpr char* idx_konsumen_nama = "CREATE INDEX idx_konsumen_nama ON konsumen(nama_lengkap);";
constexpr char* idx_konsumen_telp = "CREATE INDEX idx_konsumen_telp ON konsumen(nomor_telp);";
constexpr char* idx_konsumen_code = "CREATE INDEX idx_konsumen_code ON konsumen(customer_code);";

constexpr char* ct_product_categories = R"(
    CREATE TABLE product_categories (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        category_name TEXT NOT NULL UNIQUE,
        description TEXT,
        is_active INTEGER DEFAULT 1,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)";

constexpr char* ct_products = R"(
    CREATE TABLE products (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        sku TEXT UNIQUE NOT NULL,         -- Stock Keeping Unit
        name TEXT NOT NULL,
        category_id INTEGER,              -- TAMBAHAN: Kategori produk
        description TEXT,                 -- TAMBAHAN: Deskripsi produk
        unit TEXT DEFAULT 'pcs',          -- TAMBAHAN: Satuan (pcs, lembar, meter, dll)
        stock REAL DEFAULT 0,             -- REAL: mendukung satuan pecahan (meter, dll)
        min_stock REAL DEFAULT 0,         -- TAMBAHAN: Minimum stok untuk alert
        cost_price INTEGER DEFAULT 0,     -- TAMBAHAN: Harga pokok (HPP)
        use_area INTEGER DEFAULT 0,       -- TAMBAHAN: Apakah perhitungan harga berdasarkan Area (dipakai order_items & v_top_products)
        is_active INTEGER DEFAULT 1,      -- 1: Aktif, 0: Non-aktif
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (category_id) REFERENCES product_categories(id)
    );
)";
constexpr char* uidx_products_sku = "CREATE UNIQUE INDEX idx_products_sku ON products(sku COLLATE NOCASE);";
constexpr char* idx_products_name = "CREATE INDEX idx_products_name ON products(name);";
constexpr char* idx_products_category = "CREATE INDEX idx_products_category ON products(category_id);";

constexpr char* ct_price_levels = R"(
    CREATE TABLE price_levels (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        level_name TEXT NOT NULL UNIQUE,
        discount_percentage REAL DEFAULT 0,   -- TAMBAHAN: Persentase diskon
        description TEXT,                     -- TAMBAHAN: Deskripsi level
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)";

constexpr char* ct_product_prices = R"(
    CREATE TABLE product_prices (
        product_id INTEGER,
        price_level_id INTEGER,
        price INTEGER NOT NULL CHECK(price >= 0),
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        PRIMARY KEY (product_id, price_level_id),
        FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE,
        FOREIGN KEY (price_level_id) REFERENCES price_levels(id) ON DELETE CASCADE
    );
)";

constexpr char* ct_finishing_services = R"(
    CREATE TABLE finishing_services (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        code TEXT UNIQUE,                 -- TAMBAHAN: Kode finishing (FIN-001)
        name TEXT NOT NULL,
        description TEXT,                 -- TAMBAHAN: Deskripsi layanan
        price_per_unit INTEGER DEFAULT 0,
        unit TEXT DEFAULT 'pcs',          -- TAMBAHAN: Satuan (pcs, lembar, meter)
        is_active INTEGER DEFAULT 1,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)";
constexpr char* idx_finishing_name = "CREATE INDEX idx_finishing_name ON finishing_services(name);";

constexpr char* ct_akun_transaksi = R"(
    CREATE TABLE akun_transaksi (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        kode TEXT UNIQUE NOT NULL,            -- Kode akun: 'CASH', 'BRI', 'BCA'
        nama TEXT UNIQUE NOT NULL,            -- Nama tampilan: 'Kas Admin', 'Bank BRI'
        tipe TEXT NOT NULL COLLATE NOCASE CHECK(tipe IN (
            'cash',                           -- Uang tunai
            'bank',                           -- Rekening bank
            'ewallet',                        -- Dompet digital (OVO, DANA, GoPay)
            'lainnya'                         -- Akun lain
        )),
        nama_bank TEXT,                       -- 'BRI', 'BCA', 'Mandiri', dll
        nomor_rekening TEXT,                  -- Nomor rekening/akun
        atas_nama TEXT,                       -- Nama pemilik rekening
        saldo INTEGER DEFAULT 0,
        is_active INTEGER DEFAULT 1,
        description TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)";

constexpr char* ct_kategori_transaksi = R"(
    CREATE TABLE kategori_transaksi (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        kode TEXT UNIQUE,                 -- TAMBAHAN: Kode kategori (KAT-001)
        nama TEXT UNIQUE NOT NULL,
        tipe TEXT NOT NULL COLLATE NOCASE CHECK(tipe IN ('pemasukan', 'pengeluaran', 'opname')),
        parent_id INTEGER,                -- TAMBAHAN: Untuk sub-kategori
        description TEXT,                 -- TAMBAHAN: Deskripsi kategori
        is_active INTEGER DEFAULT 1,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (parent_id) REFERENCES kategori_transaksi(id)
    );
)";

constexpr char* ct_invoices = R"(
    CREATE TABLE invoices (
        id               INTEGER    PRIMARY KEY AUTOINCREMENT,
        invoice_number   TEXT       UNIQUE NOT NULL,

        -- Relasi
        customer_id      INTEGER,   -- sengaja untuk konsumen yang tidak terdaftar
        customer_name    TEXT       NOT NULL,
        customer_phone   TEXT,
        price_level_id   INTEGER    DEFAULT 1,
        admin_id         INTEGER    NOT NULL,

        -- Keuangan
        subtotal         INTEGER    NOT NULL DEFAULT 0,
        discount_amount  INTEGER    NOT NULL DEFAULT 0,
        tax_amount       INTEGER    NOT NULL DEFAULT 0,
        total_amount     INTEGER    GENERATED ALWAYS AS (subtotal - discount_amount + tax_amount) VIRTUAL,
        paid_amount      INTEGER    NOT NULL DEFAULT 0,
        remaining_amount INTEGER    GENERATED ALWAYS AS (total_amount - paid_amount) VIRTUAL,

        -- Status Dokumen: Fokus pada siklus hidup (Workflow)
        staging_status      TEXT    DEFAULT 'draft' COLLATE NOCASE
                                     CHECK (staging_status IN ('draft', 'issued', 'sent', 'cancelled')),

        -- Status Pembayaran: Fokus pada kas (Financial)
        settlement_status   TEXT    DEFAULT 'unpaid' COLLATE NOCASE
                                     CHECK (settlement_status IN ('unpaid', 'partial', 'paid', 'refunded')),

        -- Penanganan Revisi
        revision_no      INTEGER    DEFAULT 0,
        parent_id        INTEGER,   -- Referensi ke ID invoice sebelum direvisi
        is_active        INTEGER    DEFAULT 1 CHECK (is_active IN (0, 1)),

        -- Waktu (Semua Konsisten UTC)
        issue_date       DATETIME   DEFAULT CURRENT_TIMESTAMP,
        due_date         DATETIME,
        created_at       DATETIME   DEFAULT CURRENT_TIMESTAMP,
        updated_at       DATETIME   DEFAULT CURRENT_TIMESTAMP,

        -- Catatan
        notes            TEXT,
        internal_notes   TEXT,

        FOREIGN KEY (customer_id)    REFERENCES konsumen (id),
        FOREIGN KEY (price_level_id) REFERENCES price_levels (id),
        FOREIGN KEY (admin_id)       REFERENCES admins (id),
        FOREIGN KEY (parent_id)      REFERENCES invoices (id)
    );
)";
constexpr char* idx_invoices_number = "CREATE INDEX idx_invoices_number ON invoices(invoice_number);";
constexpr char* idx_invoices_customer = "CREATE INDEX idx_invoices_customer ON invoices(customer_id);";
constexpr char* idx_invoices_due_date = "CREATE INDEX idx_invoices_due_date ON invoices(due_date);";
constexpr char* idx_invoices_status = "CREATE INDEX idx_invoices_status ON invoices(staging_status);";
constexpr char* idx_invoices_creation = "CREATE INDEX idx_invoices_creation ON invoices(created_at);";

constexpr char* ct_orders = R"(
    CREATE TABLE orders (
        id                  INTEGER PRIMARY KEY AUTOINCREMENT,
        order_number        TEXT UNIQUE,                    -- Di-generate otomatis
        invoice_id          INTEGER,                        -- Referensi ke invoice (bisa NULL jika belum dibuat)
        invoice_number      TEXT,                           -- Denormalisasi untuk tampilan cepat

        customer_id         INTEGER,
        customer_name       TEXT NOT NULL,
        customer_phone      TEXT,
        price_level_id      INTEGER DEFAULT 1,

        -- Informasi finansial (tetap di orders untuk performa produksi)
        subtotal            INTEGER NOT NULL DEFAULT 0,
        discount_amount     INTEGER NOT NULL DEFAULT 0,
        discount_percentage REAL    DEFAULT 0,
        total_amount        INTEGER GENERATED ALWAYS AS (
                                COALESCE(subtotal,0) - COALESCE(discount_amount, 0)
                            ) VIRTUAL,

        -- Status operasional
        staging_status      TEXT DEFAULT 'pending' COLLATE NOCASE CHECK(staging_status IN ('pending','processing','ready','completed','cancelled')),
        priority            TEXT DEFAULT 'normal',

        -- Jadwal
        order_date          DATETIME DEFAULT CURRENT_TIMESTAMP,
        deadline_date       DATETIME,
        completion_date     DATETIME,

        -- Catatan
        notes               TEXT,
        internal_notes      TEXT,

        -- Tracking
        admin_id            INTEGER NOT NULL,
        created_at          DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at          DATETIME DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY (invoice_id)     REFERENCES invoices(id) ON DELETE SET NULL,
        FOREIGN KEY (customer_id)    REFERENCES konsumen(id) ON DELETE RESTRICT,
        FOREIGN KEY (admin_id)       REFERENCES admins(id) ON DELETE RESTRICT,
        FOREIGN KEY (price_level_id) REFERENCES price_levels(id)
    );
)";
constexpr char* idx_orders_invoice = "CREATE INDEX idx_orders_invoice ON orders(invoice_id);";
constexpr char* idx_orders_customer = "CREATE INDEX idx_orders_customer ON orders(customer_id);";
constexpr char* idx_orders_status = "CREATE INDEX idx_orders_status ON orders(staging_status);";
constexpr char* idx_orders_deadline = "CREATE INDEX idx_orders_deadline ON orders(deadline_date);";
constexpr char* idx_orders_creation = "CREATE INDEX idx_orders_creation ON orders(created_at);";

constexpr char* ct_order_items = R"(
    CREATE TABLE order_items (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        order_id INTEGER NOT NULL,
        product_id INTEGER,
        product_name TEXT NOT NULL,       -- TAMBAHAN: Denormalisasi nama produk
        sku TEXT,                         -- TAMBAHAN: Denormalisasi SKU
        quantity INTEGER NOT NULL DEFAULT 1
            CHECK(quantity > 0),
        unit TEXT DEFAULT 'pcs',          -- TAMBAHAN: Satuan
        size_width REAL DEFAULT 1,        -- TAMBAHAN: Panjang - hanya dihitung bila use_area = 1
        size_height REAL DEFAULT 1,       -- TAMBAHAN: Tinggi  - hanya dihitung bila use_area = 1
        use_area INTEGER DEFAULT 0,          -- TAMBAHAN: Hitung berdasar luas
        sale_price INTEGER NOT NULL,         -- Harga jual satuan
        base_price INTEGER NOT NULL,         -- Harga dasar satuan
        discount_percentage INTEGER DEFAULT 0, -- TAMBAHAN: Diskon per item (hanya estimasi tidak dihitung)
        discount_amount INTEGER DEFAULT 0,   -- TAMBAHAN: Jumlah diskon (discount real masuk hitungan)
        subtotal INTEGER GENERATED ALWAYS AS ( -- Subtotal = (quantity * sale_price * size_width * size_height)
                        CAST(((quantity * sale_price * size_width * size_height) + 99.99999) / 100 AS INT) * 100) VIRTUAL,
        finishing_total INTEGER NOT NULL DEFAULT 0,
        total INTEGER GENERATED ALWAYS AS (
                        subtotal + COALESCE(finishing_total, 0) - COALESCE(discount_amount,0) ) VIRTUAL, -- Total = Subtotal - (discount + finishing)
        notes TEXT,                          -- TAMBAHAN: Catatan khusus item
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
        FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT
    );
)";
constexpr char* idx_order_items_order = "CREATE INDEX idx_order_items_order ON order_items(order_id);";
constexpr char* idx_order_items_product = "CREATE INDEX idx_order_items_product ON order_items(product_id);";
constexpr char* idx_order_items_creation = "CREATE INDEX idx_order_items_creation ON order_items(created_at);";

constexpr char* ct_order_item_finishings = R"(
    CREATE TABLE order_item_finishings (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        order_item_id INTEGER NOT NULL,
        finishing_id INTEGER,
        finishing_name TEXT NOT NULL,     -- TAMBAHAN: Denormalisasi nama finishing
        quantity INTEGER DEFAULT 1,       -- TAMBAHAN: Jumlah yang di-finishing
        finishing_price INTEGER NOT NULL,    -- Harga finishing per unit
        subtotal        INTEGER GENERATED ALWAYS AS (quantity * finishing_price) VIRTUAL,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (order_item_id) REFERENCES order_items(id) ON UPDATE CASCADE ON DELETE CASCADE,
        FOREIGN KEY (finishing_id) REFERENCES finishing_services(id) ON UPDATE CASCADE ON DELETE RESTRICT
    );
)";
constexpr char* idx_order_finishings_item = "CREATE INDEX idx_order_finishings_item ON order_item_finishings(order_item_id);";

constexpr char* ct_payments = R"(
    CREATE TABLE payments (
        id                       INTEGER PRIMARY KEY AUTOINCREMENT,
        payment_number           TEXT UNIQUE,               -- PAY-20260320-00001
        invoice_id               INTEGER NOT NULL,          -- Referensi UTAMA (wajib)

        -- Detail pembayaran
        amount                   INTEGER NOT NULL CHECK(amount > 0),
        akun_transaksi_id        INTEGER NOT NULL,

        -- Informasi tunai
        cash_received            INTEGER,
        cash_change               INTEGER,

        -- Status & catatan
        verification_status      TEXT DEFAULT 'pending' COLLATE NOCASE CHECK( verification_status IN ('pending','verified','cancelled') ),
        notes                    TEXT,

        -- Tracking
        admin_id                 INTEGER NOT NULL,
        payment_date              DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
        verified_by               INTEGER,
        verified_at                DATETIME,

        created_at               DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at               DATETIME DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY (invoice_id)        REFERENCES invoices(id) ON DELETE RESTRICT,
        FOREIGN KEY (admin_id)          REFERENCES admins(id) ON DELETE RESTRICT,
        FOREIGN KEY (verified_by)       REFERENCES admins(id) ON DELETE RESTRICT,
        FOREIGN KEY (akun_transaksi_id) REFERENCES akun_transaksi(id) ON DELETE RESTRICT
    );
)";
constexpr char* idx_payments_admin = "CREATE INDEX idx_payments_admin ON payments(admin_id);";
constexpr char* idx_payments_invoice = "CREATE INDEX idx_payments_invoice ON payments(invoice_id);";
constexpr char* idx_payments_date = "CREATE INDEX idx_payments_date ON payments(payment_date);";
constexpr char* idx_payments_status = "CREATE INDEX idx_payments_status ON payments(verification_status);";
constexpr char* idx_payments_akun_tr = "CREATE INDEX idx_payments_akun_tr ON payments(akun_transaksi_id);";

constexpr char* ct_transaksi = R"(
    CREATE TABLE transaksi (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        akun_id INTEGER NOT NULL,
        transaction_number TEXT UNIQUE,   -- TAMBAHAN: Nomor transaksi unik
        admin_id INTEGER NOT NULL,
        kategori_id INTEGER,

        -- Detail transaksi
        tipe TEXT NOT NULL COLLATE NOCASE CHECK( tipe IN ('pemasukan', 'pengeluaran', 'opname')),
        deskripsi TEXT,

        -- Ledger Mode (Buku Besar)
        amount_before INTEGER NOT NULL,
        amount INTEGER NOT NULL,
        amount_after INTEGER NOT NULL,

        -- Informasi tambahan
        payment_method TEXT,              -- TAMBAHAN: Metode pembayaran
        reference_type TEXT,              -- TAMBAHAN: order, payment, expense
        reference_id INTEGER,             -- TAMBAHAN: ID referensi (order_id, payment_id, dll)
        attachment TEXT,                  -- TAMBAHAN: Path file lampiran (nota, bukti)

        -- Tracking
        tanggal DATE NOT NULL DEFAULT (DATE('now')),
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY (akun_id) REFERENCES akun_transaksi(id) ON DELETE RESTRICT,
        FOREIGN KEY (admin_id) REFERENCES admins(id) ON DELETE RESTRICT,
        FOREIGN KEY (kategori_id) REFERENCES kategori_transaksi(id) ON DELETE RESTRICT
    );
)";
constexpr char* idx_transaksi_admin_tanggal = "CREATE INDEX idx_transaksi_admin_tanggal ON transaksi(tanggal, admin_id);";
constexpr char* idx_transaksi_kategori = "CREATE INDEX idx_transaksi_kategori ON transaksi(kategori_id);";
constexpr char* idx_transaksi_tipe = "CREATE INDEX idx_transaksi_tipe ON transaksi(tipe);";
constexpr char* idx_transaksi_akun = "CREATE INDEX idx_transaksi_akun ON transaksi(akun_id);";
constexpr char* idx_transaksi_tanggal = "CREATE INDEX idx_transaksi_tanggal ON transaksi(tanggal);";
constexpr char* idx_transaksi_creation = "CREATE INDEX idx_transaksi_creation ON transaksi(created_at);";

constexpr char* ct_stock_movements = R"(
    CREATE TABLE stock_movements (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        product_id INTEGER NOT NULL,
        movement_type TEXT NOT NULL COLLATE NOCASE CHECK(movement_type IN ('in', 'out', 'adjustment')),
        stock_before REAL NOT NULL,       -- Stok sebelum transaksi
        quantity     REAL NOT NULL,       -- Positif untuk masuk, negatif untuk keluar
        stock_after  REAL NOT NULL,       -- Stok setelah transaksi

        -- Referensi
        reference_type TEXT,              -- order, purchase, adjustment
        reference_id INTEGER,             -- ID referensi (order_id, dll)

        -- Detail
        notes TEXT,
        admin_id INTEGER NOT NULL,
        movement_date DATETIME DEFAULT CURRENT_TIMESTAMP,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT,
        FOREIGN KEY (admin_id) REFERENCES admins(id)
    );
)";
constexpr char* idx_stock_movements_product = "CREATE INDEX idx_stock_movements_product ON stock_movements(product_id);";
constexpr char* idx_stock_product_id = "CREATE INDEX idx_stock_product_id ON stock_movements(product_id, id);";
constexpr char* idx_stock_movements_date = "CREATE INDEX idx_stock_movements_date ON stock_movements(movement_date);";
constexpr char* idx_stock_movements_type = "CREATE INDEX idx_stock_movements_type ON stock_movements(movement_type);";
constexpr char* idx_stock_movements_creation = "CREATE INDEX idx_stock_movements_creation ON stock_movements(created_at);";

constexpr char* ct_activity_logs = R"(
    CREATE TABLE activity_logs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        admin_id INTEGER,
        action TEXT NOT NULL,             -- login, create_order, update_payment, dll
        table_name TEXT,                  -- Nama tabel yang diubah
        record_id INTEGER,                -- ID record yang diubah
        old_value TEXT,                   -- Nilai lama (JSON)
        new_value TEXT,                   -- Nilai baru (JSON)
        ip_address TEXT,
        user_agent TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (admin_id) REFERENCES admins(id)
    );
)";
constexpr char* idx_activity_logs_admin = "CREATE INDEX idx_activity_logs_admin ON activity_logs(admin_id);";
constexpr char* idx_activity_logs_date = "CREATE INDEX idx_activity_logs_date ON activity_logs(created_at);";
constexpr char* idx_activity_logs_action = "CREATE INDEX idx_activity_logs_action ON activity_logs(action);";

constexpr char* ct_app_settings = R"(
    CREATE TABLE app_settings (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        setting_key TEXT UNIQUE NOT NULL,
        setting_value TEXT,
        data_type TEXT DEFAULT 'string',  -- string, number, boolean, json
        description TEXT,
        is_public INTEGER DEFAULT 0,      -- 0: hanya admin, 1: publik
        updated_by INTEGER,
        updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (updated_by) REFERENCES admins(id)
    );
)";

// ---- VIEWS - untuk laporan ----

constexpr char* vw_order_summary = R"(
    CREATE VIEW v_order_summary AS
    SELECT
        o.id,
        o.order_number,
        o.customer_name,
        o.customer_phone,
        o.total_amount,
        o.staging_status,
        o.priority,
        o.order_date,
        o.deadline_date,
        a.nama_lengkap AS admin_name,
        COUNT(DISTINCT oi.id) AS total_items,
        SUM(oi.quantity) AS total_quantity
    FROM orders o
    LEFT JOIN admins a ON o.admin_id = a.id
    LEFT JOIN order_items oi ON o.id = oi.order_id
    GROUP BY o.id;
)";

constexpr char* vw_low_stock_products = R"(
    CREATE VIEW v_low_stock_products AS
    SELECT
        p.id,
        p.sku,
        p.name,
        pc.category_name,
        p.stock,
        p.min_stock,
        (p.min_stock - p.stock) AS deficit
    FROM products p
    LEFT JOIN product_categories pc ON p.category_id = pc.id
    WHERE p.is_active = 1 AND p.stock <= p.min_stock;
)";

constexpr char* vw_daily_sales = R"(
    CREATE VIEW v_daily_sales AS
    SELECT
        DATE(o.order_date) AS sale_date,
        COUNT(DISTINCT o.id) AS total_orders,
        SUM(o.total_amount) AS total_sales,
        COUNT(DISTINCT o.customer_id) AS unique_customers
    FROM orders o
    WHERE o.staging_status != 'cancelled'
    GROUP BY DATE(o.order_date);
)";

constexpr char* vw_top_products = R"(
    CREATE VIEW v_top_products AS
    SELECT p.id,
           p.sku,
           p.name,
           pc.category_name,
           COUNT(oi.id) AS order_count,
           CASE WHEN p.use_area = 0 THEN SUM(oi.quantity) ELSE SUM(oi.quantity * oi.size_width * oi.size_height) END AS total_sold,
           SUM(oi.subtotal) AS total_revenue
      FROM products p
           LEFT JOIN
           product_categories pc ON p.category_id = pc.id
           LEFT JOIN
           order_items oi ON p.id = oi.product_id
           LEFT JOIN
           orders o ON oi.order_id = o.id
     WHERE o.staging_status != 'cancelled'
     GROUP BY p.id
     ORDER BY total_revenue DESC;
)";

constexpr char* vw_top_customers = R"(
    CREATE VIEW v_top_customers AS
        SELECT k.id,
               k.customer_code,
               k.nama_lengkap,
               k.nomor_telp,
               COUNT(o.id) AS total_orders,
               COALESCE(SUM(o.total_amount), 0) AS total_spent,
               k.last_seen,
               CASE WHEN COALESCE(SUM(o.total_amount), 0) >= 10000000 THEN 'VIP'
                    WHEN COALESCE(SUM(o.total_amount), 0) >= 5000000  THEN 'Gold'
                    WHEN COALESCE(SUM(o.total_amount), 0) >= 1000000  THEN 'Silver'
                    ELSE 'Regular' END AS customer_tier
          FROM konsumen k
               LEFT JOIN
               orders o ON o.customer_id = k.id AND
                           o.staging_status != 'cancelled' -- jangan hitung order batal
         WHERE k.is_active = 1
         GROUP BY k.id,
                  k.customer_code,
                  k.nama_lengkap,
                  k.nomor_telp,
                  k.last_seen
         ORDER BY total_spent DESC;
)";

// ---- Daftar statement V1, siap di-iterasi berurutan ----
// Cara pakai (contoh):
//     exec(SQLITE_SCHEMA::FK_OFF);
//     exec(SQLITE_SCHEMA::TR_BEGIN);
//     for (const char* stmt : SQLITE_SCHEMA::V1::MigrateV1) {
//         if (!exec(stmt)) { exec(SQLITE_SCHEMA::ROLLBACK); break; }
//     }
//     exec(SQLITE_SCHEMA::COMMIT);
//     exec(SQLITE_SCHEMA::FK_ON);
//
// Urutan sudah mengikuti dependensi FOREIGN KEY antar tabel (tabel yang
// direferensikan dibuat lebih dulu), jadi aman dieksekusi meski FK_ON.
// FK_OFF/TR_BEGIN tetap disarankan agar migrasi bersifat atomik.
constexpr const char* MigrateV1[] = {
    ct_roles,
    ct_admins, idx_admins_username, idx_admins_role,
    ct_product_categories,
    ct_products, uidx_products_sku, idx_products_name, idx_products_category,
    ct_price_levels,
    ct_konsumen, idx_konsumen_nama, idx_konsumen_telp, idx_konsumen_code,
    ct_product_prices,
    ct_finishing_services, idx_finishing_name,
    ct_akun_transaksi,
    ct_kategori_transaksi,
    ct_invoices, idx_invoices_number, idx_invoices_customer, idx_invoices_due_date, idx_invoices_status, idx_invoices_creation,
    ct_orders, idx_orders_invoice, idx_orders_customer, idx_orders_status, idx_orders_deadline, idx_orders_creation,
    ct_order_items, idx_order_items_order, idx_order_items_product, idx_order_items_creation,
    ct_order_item_finishings, idx_order_finishings_item,
    ct_payments, idx_payments_admin, idx_payments_invoice, idx_payments_date, idx_payments_status, idx_payments_akun_tr,
    ct_transaksi, idx_transaksi_admin_tanggal, idx_transaksi_kategori, idx_transaksi_tipe, idx_transaksi_akun, idx_transaksi_tanggal, idx_transaksi_creation,
    ct_stock_movements, idx_stock_movements_product, idx_stock_product_id, idx_stock_movements_date, idx_stock_movements_type, idx_stock_movements_creation,
    ct_activity_logs, idx_activity_logs_admin, idx_activity_logs_date, idx_activity_logs_action,
    ct_app_settings,
    // Views dibuat paling akhir karena mereferensikan banyak tabel sekaligus
    vw_order_summary,
    vw_low_stock_products,
    vw_daily_sales,
    vw_top_products,
    vw_top_customers,
};
constexpr size_t MigrateV1Count = sizeof(MigrateV1) / sizeof(MigrateV1[0]);

} // NS V1
} // NS SQLITE_SCHEMA

