#pragma once

#include <cstddef>

namespace Migration {

    constexpr const char* FK_OFF = "PRAGMA foreign_keys = OFF;";
    constexpr const char* FK_ON = "PRAGMA foreign_keys = ON;";
    constexpr const char* BEGIN_TRANSACTION = "BEGIN TRANSACTION;";
    constexpr const char* COMMIT = "COMMIT;";
    constexpr const char* ROLLBACK = "ROLLBACK;";

    namespace SqliteV1 {

        // ====================================================================
        // 1. CREATE TABLES
        // ====================================================================

        constexpr const char* ct_roles = R"(
            CREATE TABLE roles (
                id INTEGER PRIMARY KEY,
                role_name TEXT NOT NULL UNIQUE,
                description TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )";

        constexpr const char* ct_admins = R"(
            CREATE TABLE admins (
                id INTEGER PRIMARY KEY,
                role_id INTEGER NOT NULL DEFAULT 3,
                username TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                salt TEXT NOT NULL,
                nama_lengkap TEXT NOT NULL,
                email TEXT,
                nomor_telp TEXT,
                is_active INTEGER DEFAULT 1,
                last_login DATETIME,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE RESTRICT
            );
        )";

        constexpr const char* ct_price_levels = R"(
            CREATE TABLE price_levels (
                id INTEGER PRIMARY KEY,
                level_name TEXT NOT NULL UNIQUE,
                discount_percentage REAL DEFAULT 0,
                description TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )";

        constexpr const char* ct_konsumen = R"(
            CREATE TABLE konsumen (
                id INTEGER PRIMARY KEY,
                customer_code TEXT UNIQUE,
                nama_lengkap TEXT NOT NULL,
                customer_type TEXT DEFAULT 'Individual',
                email TEXT,
                nomor_telp TEXT,
                alamat TEXT,
                kota TEXT,
                kode_pos TEXT,
                npwp TEXT,
                catatan TEXT,
                price_level_id INTEGER DEFAULT 1,
                is_active INTEGER DEFAULT 1,
                last_seen DATETIME,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (price_level_id) REFERENCES price_levels(id)
            );
        )";

        constexpr const char* ct_product_categories = R"(
            CREATE TABLE product_categories (
                id INTEGER PRIMARY KEY,
                category_name TEXT NOT NULL UNIQUE,
                description TEXT,
                is_active INTEGER DEFAULT 1,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )";

        constexpr const char* ct_products = R"(
            CREATE TABLE products (
                id INTEGER PRIMARY KEY,
                sku TEXT UNIQUE NOT NULL,
                name TEXT NOT NULL,
                category_id INTEGER,
                description TEXT,
                unit TEXT COLLATE NOCASE DEFAULT 'pcs',
                stock REAL DEFAULT 0,
                min_stock REAL DEFAULT 0,
                cost_price INTEGER DEFAULT 0,
                use_area INTEGER DEFAULT 0,
                is_active INTEGER DEFAULT 1,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (category_id) REFERENCES product_categories(id)
            );
        )";

        constexpr const char* ct_product_prices = R"(
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

        constexpr const char* ct_finishing_services = R"(
            CREATE TABLE finishing_services (
                id INTEGER PRIMARY KEY,
                code TEXT UNIQUE,
                name TEXT NOT NULL,
                description TEXT,
                price_per_unit INTEGER DEFAULT 0,
                unit TEXT DEFAULT 'pcs',
                is_active INTEGER DEFAULT 1,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )";

        constexpr const char* ct_invoices = R"(
            CREATE TABLE invoices (
                id               INTEGER    PRIMARY KEY AUTOINCREMENT,
                invoice_number   TEXT       UNIQUE NOT NULL,
                customer_id      INTEGER,
                customer_name    TEXT       NOT NULL,
                customer_phone   TEXT,
                price_level_id   INTEGER    DEFAULT 1,
                admin_id         INTEGER    NOT NULL,
                subtotal         INTEGER    NOT NULL DEFAULT 0,
                discount_amount  INTEGER    NOT NULL DEFAULT 0,
                tax_amount       INTEGER    NOT NULL DEFAULT 0,
                total_amount     INTEGER    GENERATED ALWAYS AS (subtotal - discount_amount + tax_amount) VIRTUAL,
                paid_amount      INTEGER    NOT NULL DEFAULT 0,
                remaining_amount INTEGER    GENERATED ALWAYS AS (total_amount - paid_amount) VIRTUAL,
                staging_status      TEXT    DEFAULT 'draft' COLLATE NOCASE 
                                            CHECK (staging_status IN ('draft', 'issued', 'sent', 'cancelled')),
                settlement_status   TEXT    DEFAULT 'unpaid' COLLATE NOCASE 
                                            CHECK (settlement_status IN ('unpaid', 'partial', 'paid', 'refunded')),
                revision_no      INTEGER    DEFAULT 0,
                parent_id        INTEGER,
                is_active        INTEGER    DEFAULT 1 CHECK (is_active IN (0, 1)),
                issue_date       DATETIME   DEFAULT CURRENT_TIMESTAMP,
                due_date         DATETIME,
                created_at       DATETIME   DEFAULT CURRENT_TIMESTAMP,
                updated_at       DATETIME   DEFAULT CURRENT_TIMESTAMP,
                notes            TEXT,
                internal_notes   TEXT,
                FOREIGN KEY (customer_id)    REFERENCES konsumen (id),
                FOREIGN KEY (price_level_id) REFERENCES price_levels (id),
                FOREIGN KEY (admin_id)       REFERENCES admins (id),
                FOREIGN KEY (parent_id)      REFERENCES invoices (id)
            );
        )";

        constexpr const char* ct_orders = R"(
            CREATE TABLE orders (
                id                  INTEGER PRIMARY KEY,
                order_number        TEXT UNIQUE,
                invoice_id          INTEGER,
                invoice_number      TEXT,
                customer_id         INTEGER,
                customer_name       TEXT NOT NULL,
                customer_phone      TEXT,
                price_level_id      INTEGER DEFAULT 1,
                subtotal            INTEGER NOT NULL DEFAULT 0,
                discount_amount     INTEGER NOT NULL DEFAULT 0,
                discount_percentage REAL    DEFAULT 0,
                total_amount        INTEGER GENERATED ALWAYS AS (
                                        COALESCE(subtotal,0) - COALESCE(discount_amount, 0)
                                    ) VIRTUAL,
                staging_status      TEXT DEFAULT 'pending' COLLATE NOCASE CHECK(staging_status IN ('pending','processing','ready','completed','cancelled')),
                priority            TEXT DEFAULT 'normal',
                order_date          DATETIME DEFAULT CURRENT_TIMESTAMP,
                deadline_date       DATETIME,
                completion_date     DATETIME,
                notes               TEXT,
                internal_notes      TEXT,
                admin_id            INTEGER NOT NULL,
                created_at          DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at          DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (invoice_id)     REFERENCES invoices(id) ON DELETE SET NULL,
                FOREIGN KEY (customer_id)    REFERENCES konsumen(id) ON DELETE RESTRICT,
                FOREIGN KEY (admin_id)       REFERENCES admins(id) ON DELETE RESTRICT,
                FOREIGN KEY (price_level_id) REFERENCES price_levels(id)
            );
        )";

        constexpr const char* ct_order_items = R"(
            CREATE TABLE order_items (
                id INTEGER PRIMARY KEY,
                order_id INTEGER NOT NULL,
                product_id INTEGER,
                product_name TEXT NOT NULL,
                sku TEXT,
                quantity INTEGER NOT NULL DEFAULT 1 CHECK(quantity > 0),
                unit TEXT DEFAULT 'pcs',
                size_width REAL DEFAULT 1,
                size_height REAL DEFAULT 1,
                use_area INTEGER DEFAULT 0,
                sale_price INTEGER NOT NULL,
                base_price INTEGER NOT NULL,
                discount_percentage INTEGER DEFAULT 0,
                discount_amount INTEGER DEFAULT 0,
                subtotal INTEGER GENERATED ALWAYS AS (
                                CAST(((quantity * sale_price * size_width * size_height) + 99.99999) / 100 AS INT) * 100) VIRTUAL,
                finishing_total INTEGER NOT NULL DEFAULT 0,
                total INTEGER GENERATED ALWAYS AS (
                                subtotal + COALESCE(finishing_total, 0) - COALESCE(discount_amount,0) ) VIRTUAL,
                notes TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
                FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT
            );
        )";

        constexpr const char* ct_order_item_finishings = R"(
            CREATE TABLE order_item_finishings (
                id INTEGER PRIMARY KEY,
                order_item_id INTEGER NOT NULL,
                finishing_id INTEGER,
                finishing_name TEXT NOT NULL,
                quantity INTEGER DEFAULT 1,
                finishing_price INTEGER NOT NULL,
                subtotal        INTEGER GENERATED ALWAYS AS (quantity * finishing_price) VIRTUAL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (order_item_id) REFERENCES order_items(id) ON UPDATE CASCADE ON DELETE CASCADE,
                FOREIGN KEY (finishing_id) REFERENCES finishing_services(id) ON UPDATE CASCADE ON DELETE RESTRICT
            );
        )";

        constexpr const char* ct_akun_transaksi = R"(
            CREATE TABLE akun_transaksi (
                id INTEGER PRIMARY KEY,
                kode TEXT UNIQUE NOT NULL,
                nama TEXT UNIQUE NOT NULL,
                tipe TEXT NOT NULL COLLATE NOCASE CHECK(tipe IN ('cash', 'bank', 'ewallet', 'lainnya')),
                nama_bank TEXT,
                nomor_rekening TEXT,
                atas_nama TEXT,
                saldo INTEGER DEFAULT 0,
                is_active INTEGER DEFAULT 1,
                description TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )";

        constexpr const char* ct_payments = R"(
            CREATE TABLE payments (
                id                       INTEGER PRIMARY KEY,
                payment_number           TEXT UNIQUE,
                invoice_id               INTEGER NOT NULL,
                amount                   INTEGER NOT NULL CHECK(amount > 0),
                akun_transaksi_id        INTEGER NOT NULL,
                cash_received            INTEGER,
                cash_change              INTEGER,
                verification_status      TEXT DEFAULT 'pending' COLLATE NOCASE CHECK( verification_status IN ('pending','verified','cancelled') ),
                notes                    TEXT,
                admin_id                 INTEGER NOT NULL,
                payment_date             DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
                verified_by              INTEGER,
                verified_at              DATETIME,
                created_at               DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at               DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (invoice_id)        REFERENCES invoices(id) ON DELETE RESTRICT,
                FOREIGN KEY (admin_id)          REFERENCES admins(id) ON DELETE RESTRICT,
                FOREIGN KEY (verified_by)       REFERENCES admins(id) ON DELETE RESTRICT,
                FOREIGN KEY (akun_transaksi_id) REFERENCES akun_transaksi(id) ON DELETE RESTRICT
            );
        )";

        constexpr const char* ct_kategori_transaksi = R"(
            CREATE TABLE kategori_transaksi (
                id INTEGER PRIMARY KEY,
                kode TEXT UNIQUE,
                nama TEXT UNIQUE NOT NULL,
                tipe TEXT NOT NULL COLLATE NOCASE CHECK( tipe IN ('pemasukan', 'pengeluaran', 'opname')),
                parent_id INTEGER,
                description TEXT,
                is_active INTEGER DEFAULT 1,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (parent_id) REFERENCES kategori_transaksi(id)
            );
        )";

        constexpr const char* ct_transaksi = R"(
            CREATE TABLE transaksi (
                id INTEGER PRIMARY KEY,
                akun_id INTEGER NOT NULL,
                transaction_number TEXT UNIQUE,
                admin_id INTEGER NOT NULL,
                kategori_id INTEGER,
                tipe TEXT NOT NULL COLLATE NOCASE CHECK( tipe IN ('pemasukan', 'pengeluaran', 'opname')),
                deskripsi TEXT,
                amount_before INT NOT NULL,
                amount INT NOT NULL,
                amount_after INT NOT NULL,
                payment_method TEXT,
                reference_type TEXT,
                reference_id INTEGER,
                attachment TEXT,
                tanggal DATE NOT NULL DEFAULT (DATE('now')),
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (akun_id) REFERENCES akun_transaksi(id) ON DELETE RESTRICT,
                FOREIGN KEY (admin_id) REFERENCES admins(id) ON DELETE RESTRICT,
                FOREIGN KEY (kategori_id) REFERENCES kategori_transaksi(id) ON DELETE RESTRICT
            );
        )";

        constexpr const char* ct_stock_movements = R"(
            CREATE TABLE stock_movements (
                id INTEGER PRIMARY KEY,
                product_id INTEGER NOT NULL,
                movement_type TEXT NOT NULL COLLATE NOCASE CHECK(movement_type IN ('in', 'out', 'adjustment')),
                stock_before REAL NOT NULL,
                quantity     REAL NOT NULL,
                stock_after  REAL NOT NULL,
                reference_type TEXT,
                reference_id INTEGER,
                notes TEXT,
                admin_id INTEGER NOT NULL,
                movement_date DATETIME DEFAULT CURRENT_TIMESTAMP,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT,
                FOREIGN KEY (admin_id) REFERENCES admins(id)
            );
        )";

        constexpr const char* ct_activity_logs = R"(
            CREATE TABLE activity_logs (
                id INTEGER PRIMARY KEY,
                admin_id INTEGER,
                action TEXT NOT NULL,
                table_name TEXT,
                record_id INTEGER,
                old_value TEXT,
                new_value TEXT,
                ip_address TEXT,
                user_agent TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (admin_id) REFERENCES admins(id)
            );
        )";

        constexpr const char* ct_app_settings = R"(
            CREATE TABLE app_settings (
                id INTEGER PRIMARY KEY,
                setting_key TEXT UNIQUE NOT NULL,
                setting_value TEXT,
                data_type TEXT DEFAULT 'string',
                description TEXT,
                is_public INTEGER DEFAULT 0,
                updated_by INTEGER,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (updated_by) REFERENCES admins(id)
            );
        )";

        // ====================================================================
        // 2. CREATE INDEXES
        // ====================================================================

        constexpr const char* idx_admins_username = "CREATE INDEX idx_admins_username ON admins(username);";
        constexpr const char* idx_admins_role = "CREATE INDEX idx_admins_role ON admins(role_id);";

        constexpr const char* idx_konsumen_nama = "CREATE INDEX idx_konsumen_nama ON konsumen(nama_lengkap);";
        constexpr const char* idx_konsumen_telp = "CREATE INDEX idx_konsumen_telp ON konsumen(nomor_telp);";
        constexpr const char* idx_konsumen_code = "CREATE INDEX idx_konsumen_code ON konsumen(customer_code);";

        constexpr const char* uidx_products_sku = "CREATE UNIQUE INDEX idx_products_sku ON products(sku COLLATE NOCASE);";
        constexpr const char* idx_products_name = "CREATE INDEX idx_products_name ON products(name);";
        constexpr const char* idx_products_category = "CREATE INDEX idx_products_category ON products(category_id);";

        constexpr const char* idx_finishing_name = "CREATE INDEX idx_finishing_name ON finishing_services(name);";

        constexpr const char* idx_orders_invoice = "CREATE INDEX idx_orders_invoice ON orders(invoice_id);";
        constexpr const char* idx_orders_customer = "CREATE INDEX idx_orders_customer ON orders(customer_id);";
        constexpr const char* idx_orders_status = "CREATE INDEX idx_orders_status ON orders(staging_status);";
        constexpr const char* idx_orders_deadline = "CREATE INDEX idx_orders_deadline ON orders(deadline_date);";
        constexpr const char* idx_orders_creation = "CREATE INDEX idx_orders_creation ON orders(created_at);";

        constexpr const char* idx_order_items_order = "CREATE INDEX idx_order_items_order ON order_items(order_id);";
        constexpr const char* idx_order_items_product = "CREATE INDEX idx_order_items_product ON order_items(product_id);";
        constexpr const char* idx_order_items_creation = "CREATE INDEX idx_order_items_creation ON order_items(created_at);";

        constexpr const char* idx_order_finishings_item = "CREATE INDEX idx_order_finishings_item ON order_item_finishings(order_item_id);";

        constexpr const char* idx_payments_admin = "CREATE INDEX idx_payments_admin ON payments(admin_id);";
        constexpr const char* idx_payments_invoice = "CREATE INDEX idx_payments_invoice ON payments(invoice_id);";
        constexpr const char* idx_payments_date = "CREATE INDEX idx_payments_date ON payments(payment_date);";
        constexpr const char* idx_payments_status = "CREATE INDEX idx_payments_status ON payments(verification_status);";
        constexpr const char* idx_payments_akun_tr = "CREATE INDEX idx_payments_akun_tr ON payments(akun_transaksi_id);";

        constexpr const char* idx_transaksi_admin_tanggal = "CREATE INDEX idx_transaksi_admin_tanggal ON transaksi(tanggal, admin_id);";
        constexpr const char* idx_transaksi_kategori = "CREATE INDEX idx_transaksi_kategori ON transaksi(kategori_id);";
        constexpr const char* idx_transaksi_tipe = "CREATE INDEX idx_transaksi_tipe ON transaksi(tipe);";
        constexpr const char* idx_transaksi_akun = "CREATE INDEX idx_transaksi_akun ON transaksi(akun_id);";
        constexpr const char* idx_transaksi_tanggal = "CREATE INDEX idx_transaksi_tanggal ON transaksi(tanggal);";
        constexpr const char* idx_transaksi_creation = "CREATE INDEX idx_transaksi_creation ON transaksi(created_at);";

        constexpr const char* idx_stock_movements_product = "CREATE INDEX idx_stock_movements_product ON stock_movements(product_id);";
        constexpr const char* idx_stock_product_id = "CREATE INDEX idx_stock_product_id ON stock_movements(product_id, id);";
        constexpr const char* idx_stock_movements_date = "CREATE INDEX idx_stock_movements_date ON stock_movements(movement_date);";
        constexpr const char* idx_stock_movements_type = "CREATE INDEX idx_stock_movements_type ON stock_movements(movement_type);";
        constexpr const char* idx_stock_movements_creation = "CREATE INDEX idx_stock_movements_creation ON stock_movements(created_at);";

        constexpr const char* idx_activity_logs_admin = "CREATE INDEX idx_activity_logs_admin ON activity_logs(admin_id);";
        constexpr const char* idx_activity_logs_date = "CREATE INDEX idx_activity_logs_date ON activity_logs(created_at);";
        constexpr const char* idx_activity_logs_action = "CREATE INDEX idx_activity_logs_action ON activity_logs(action);";

        constexpr const char* idx_invoices_number = "CREATE INDEX idx_invoices_number ON invoices(invoice_number);";
        constexpr const char* idx_invoices_customer = "CREATE INDEX idx_invoices_customer ON invoices(customer_id);";
        constexpr const char* idx_invoices_due_date = "CREATE INDEX idx_invoices_due_date ON invoices(due_date);";
        constexpr const char* idx_invoices_status = "CREATE INDEX idx_invoices_status ON invoices(staging_status);";
        constexpr const char* idx_invoices_creation = "CREATE INDEX idx_invoices_creation ON invoices(created_at);";

        // ====================================================================
        // 3. CREATE VIEWS
        // ====================================================================

        constexpr const char* cv_order_summary = R"(
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

        constexpr const char* cv_low_stock_products = R"(
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

        constexpr const char* cv_daily_sales = R"(
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

        constexpr const char* cv_top_products = R"(
            CREATE VIEW v_top_products AS
            SELECT p.id,
                   p.sku,
                   p.name,
                   pc.category_name,
                   COUNT(oi.id) AS order_count,
                   CASE WHEN p.use_area = 0 THEN SUM(oi.quantity) ELSE SUM(oi.quantity * oi.size_width * oi.size_height) END AS total_sold,
                   SUM(oi.subtotal) AS total_revenue
              FROM products p
                   LEFT JOIN product_categories pc ON p.category_id = pc.id
                   LEFT JOIN order_items oi ON p.id = oi.product_id
                   LEFT JOIN orders o ON oi.order_id = o.id
             WHERE o.staging_status != 'cancelled'
             GROUP BY p.id
             ORDER BY total_revenue DESC;
        )";

        constexpr const char* cv_top_customers = R"(
            CREATE VIEW v_top_customers AS
                SELECT k.id,
                       k.customer_code,
                       k.nama_lengkap,
                       k.nomor_telp,
                       COUNT(o.id) AS total_orders,
                       COALESCE(SUM(o.total_amount), 0) AS total_spent,
                       k.last_seen,
                       CASE WHEN COALESCE(SUM(o.total_amount), 0) >= 10000000 THEN 'VIP' WHEN COALESCE(SUM(o.total_amount), 0) >= 5000000 THEN 'Gold' WHEN COALESCE(SUM(o.total_amount), 0) >= 1000000 THEN 'Silver' ELSE 'Regular' END AS customer_tier
                  FROM konsumen k
                       LEFT JOIN orders o ON o.customer_id = k.id AND o.staging_status != 'cancelled'
                 WHERE k.is_active = 1
                 GROUP BY k.id, k.customer_code, k.nama_lengkap, k.nomor_telp, k.last_seen
                 ORDER BY total_spent DESC;
        )";

        // ====================================================================
        // 4. DATA INITIALIZATION
        // ====================================================================

        constexpr const char* init_app_settings = R"(
            INSERT INTO app_settings (setting_key, setting_value, data_type, description) VALUES
            ('company_name', 'Percetakan Maju Jaya', 'string', 'Nama perusahaan'),
            ('company_address', '', 'string', 'Alamat perusahaan'),
            ('company_phone', '', 'string', 'Nomor telepon perusahaan'),
            ('company_email', '', 'string', 'Email perusahaan'),
            ('tax_percentage', '11', 'number', 'Persentase PPN'),
            ('currency', 'IDR', 'string', 'Mata uang'),
            ('order_number_prefix', 'ORD', 'string', 'Prefix nomor order'),
            ('payment_number_prefix', 'PAY', 'string', 'Prefix nomor pembayaran'),
            ('auto_complete_paid_orders', '1', 'boolean', 'Otomatis selesaikan order yang lunas'),
            ('low_stock_alert', '10', 'number', 'Alert jika stok dibawah nilai ini'),
            ('max_login_failCount', '3', 'number', 'Jeda login jika gagal melewati batas ini'),
            ('login_failCount_timeout_sec', '30', 'number', 'Batas waktu agar bisa relogin setelah failCount');
        )";

        constexpr const char* init_finishing_services = R"(
            INSERT INTO finishing_services (code, name, description, price_per_unit) VALUES 
            ('A3DOF', 'L DOFF', 'A3Plus Laminasi Doff', 4000),
            ('A3GLS', 'L GLOS', 'A3Plus Laminasi Glossy', 4000),
            ('A3CSUMA', 'CUT SUMMA', 'A3Plus Cutting - Summa', 5000),
            ('A3CSAGA', 'CUT SAGA', 'A3Plus Cutting - SAGA', 4000),
            ('A3PTGA', 'A3PTG BS', 'A3Plus Potong Uk Besar', 4000),
            ('A3PTGB', 'A3PTG SD', 'A3Plus Potong Uk Sedang', 8000),
            ('A3PTGC', 'A3PTG KC', 'A3Plus Potong Uk Kecil', 12000);
        )";

        constexpr const char* init_roles = R"(
            INSERT INTO roles (id, role_name, description) VALUES
            (1, 'super_admin', 'Akses penuh ke seluruh sistem'),
            (2, 'kasir', 'Menangani transaksi dan pembayaran'),
            (3, 'operator', 'Mengelola order dan produksi');
        )";

        constexpr const char* init_kategori_transaksi = R"(
            INSERT INTO kategori_transaksi (nama, tipe, description, parent_id) VALUES
            ('Pemasukan',        'pemasukan',   'Kategori parent pemasukan',       NULL),
            ('Pengeluaran',      'pengeluaran', 'Kategori parent pengeluaran',     NULL),
            ('Opname',           'opname',      'Kategori parent opname',          NULL),
            ('Penjualan Produk', 'pemasukan',   'Pemasukan dari penjualan produk', 1),
            ('Penjualan Jasa',   'pemasukan',   'Pemasukan dari penjualan jasa',   2);
        )";

        constexpr const char* init_price_levels = R"(
            INSERT INTO price_levels (id, level_name, discount_percentage, description) VALUES
            (1, 'ORDER', 0, 'Harga minimum untuk pelanggan OD'),
            (2, 'MAKLOON', 0, 'Harga minimum untuk reseller MAKLOON'),
            (3, 'NEGO', 0, 'Harga Penawarn untuk order banyak');
        )";

        constexpr const char* init_product_categories = R"(
            INSERT INTO product_categories (category_name, description) VALUES
            ('LargeFormat', 'Banner, Spanduk dan Large Format lainnya'),
            ('A3Plus',      'Brosur, Sticker, Flyer, dan Cetak menggunakan mesin A3 Plus'),
            ('CTP',         'Cetak CTP untuk percetakan offset'),
            ('LASER',       'Laser Cutting Akrilik, Kayu, MDF, dan lainnya'),
            ('Offset',      'Cetak Offset untuk Undangan, Kartu Nama dalam jumlah besar dan Cetak Offset lainnya');
        )";

        constexpr const char* init_products = R"(
            INSERT INTO products (sku, name, category_id, description, unit, stock, min_stock, cost_price, use_area) VALUES 
            ('BN-FLEX',  'FLEXY',           1, 'Cetak Banner Bahan Fleksi',           'meter',   120, 30,  15000,  1),
            ('BN-KOR',   'KOREA',           1, 'Cetak Banner Bahan Korea',            'meter',   80,  20,  45000,  1),
            ('IND-KOR',  'Indoor KOREA',    1, 'Cetak Printer Indoor Bahan Korea',    'meter',   60,  15,  90000,  1),
            ('IND-GFT',  'Indoor Graftack', 1, 'Cetak Printer Indoor Bahan Graftack', 'meter',   50,  10,  90000,  1),
            ('IND-LST',  'Indoor Luster',   1, 'Cetak Printer Indoor Bahan Luster',   'meter',   40,  10,  120000, 1),
            ('A3AP15',   'AP150',           2, 'Cetak A3+ Bahan AP150',               'lembar',  200, 50,  2500,   0),
            ('A3AP21',   'AP210',           2, 'Cetak A3+ Bahan AP210',               'lembar',  150, 40,  3000,   0),
            ('A3AP23',   'AP230',           2, 'Cetak A3+ Bahan AP230',               'lembar',  100, 25,  3000,   0),
            ('A3AP26',   'AP260',           2, 'Cetak A3+ Bahan AP260',               'lembar',  90,  20,  3000,   0),
            ('A3AP26BB', 'AP260 BB',        2, 'Cetak A3+ Bahan AP260 2Sisi',         'lembar',  80,  20,  5000,   0),
            ('A3VNL',    'VINYL',           2, 'Cetak A3+ Bahan VINYL',               'lembar',  70,  15,  8500,   0),
            ('A3TRNS',   'TRANSPARENT',     2, 'Cetak A3+ Bahan TRANSPARENT',         'lembar',  60,  15,  8500,   0),
            ('A3PVC',    'PVC',             2, 'Cetak A3+ Bahan PVC',                 'set',     50,  10,  75000,  0),
            ('A3PVCNF',  'PVCNF',           2, 'Cetak A3+ Bahan PVC Tanpa finishing', 'set',     40,  10,  60000,  0),
            ('A3HVS',    'HVS',             2, 'Cetak A3+ Bahan HVS',                 'lembar',  200, 50,  2500,   0),
            ('A3KAL+',   'KALKIR',          2, 'Cetak A3+ Bahan KALKIR',              'lembar',  30,  10,  10000,  0),
            ('CTPTOKO',  'Toko',            3, 'Pelat Toko',                          'pcs',     20,  5,   12000,  0),
            ('CTPSORM',  'SORM',            3, 'Pelat SORM',                          'pcs',     15,  5,   20000,  0),
            ('CTPP46',   'P46',             3, 'Pelat 46',                            'pcs',     10,  3,   15000,  0),
            ('CTPP52',   'P52',             3, 'Pelat 52',                            'pcs',     8,   2,   35000,  0),
            ('OFFTOKO',  'COFFTOKO',        5, 'Cetak Offset Toko',                   'set',     100, 25,  12000,  0),
            ('OFFSORMF', 'COFFSORMF',       5, 'Cetak Offset SORM Full Color',        'set',     80,  20,  20000,  0),
            ('OFFP46F',  'COFFP46F',        5, 'Cetak Offset P46 Full Color',         'set',     60,  15,  15000,  0),
            ('OFFP52F',  'COFFP52F',        5, 'Cetak Offset P52 Full Color',         'set',     40,  10,  35000,  0);
        )";

        constexpr const char* init_konsumen = R"(
            INSERT INTO konsumen (customer_code, nama_lengkap, customer_type, email, nomor_telp, alamat, kota, kode_pos, npwp, catatan, price_level_id) VALUES
            ('Tamu', 'Guest', 'Individual', NULL, NULL, NULL, NULL, NULL, NULL, 'Semua Pelanggan Belum terdaftar', 1);
        )";

        constexpr const char* init_akun_transaksi = R"(
            INSERT INTO akun_transaksi ( kode, nama, tipe, nama_bank, nomor_rekening, atas_nama, saldo, description ) VALUES 
            ( 'CASH', 'Kas Admin', 'cash', NULL, NULL, NULL, 0, 'Akun trasaksi default' );
        )";

        // ====================================================================
        // 5. QUERY EXECUTION ARRAY
        // ====================================================================
        
        // Disusun berdasarkan urutan dependensi (Foreign Key rules)
        constexpr const char* queries[] = {
            // Tables
            ct_roles,
            ct_admins,
            ct_price_levels,
            ct_konsumen,
            ct_product_categories,
            ct_products,
            ct_product_prices,
            ct_finishing_services,
            ct_invoices,
            ct_orders,
            ct_order_items,
            ct_order_item_finishings,
            ct_akun_transaksi,
            ct_payments,
            ct_kategori_transaksi,
            ct_transaksi,
            ct_stock_movements,
            ct_activity_logs,
            ct_app_settings,

            // Indexes
            idx_admins_username,
            idx_admins_role,
            idx_konsumen_nama,
            idx_konsumen_telp,
            idx_konsumen_code,
            uidx_products_sku,
            idx_products_name,
            idx_products_category,
            idx_finishing_name,
            idx_orders_invoice,
            idx_orders_customer,
            idx_orders_status,
            idx_orders_deadline,
            idx_orders_creation,
            idx_order_items_order,
            idx_order_items_product,
            idx_order_items_creation,
            idx_order_finishings_item,
            idx_payments_admin,
            idx_payments_invoice,
            idx_payments_date,
            idx_payments_status,
            idx_payments_akun_tr,
            idx_transaksi_admin_tanggal,
            idx_transaksi_kategori,
            idx_transaksi_tipe,
            idx_transaksi_akun,
            idx_transaksi_tanggal,
            idx_transaksi_creation,
            idx_stock_movements_product,
            idx_stock_product_id,
            idx_stock_movements_date,
            idx_stock_movements_type,
            idx_stock_movements_creation,
            idx_activity_logs_admin,
            idx_activity_logs_date,
            idx_activity_logs_action,
            idx_invoices_number,
            idx_invoices_customer,
            idx_invoices_due_date,
            idx_invoices_status,
            idx_invoices_creation,

            // Views
            cv_order_summary,
            cv_low_stock_products,
            cv_daily_sales,
            cv_top_products,
            cv_top_customers,

            // Inits
            init_app_settings,
            init_finishing_services,
            init_roles,
            init_kategori_transaksi,
            init_price_levels,
            init_product_categories,
            init_products,
            init_konsumen,
            init_akun_transaksi
        };

        constexpr size_t queries_count = sizeof(queries) / sizeof(queries[0]);

    } // namespace SqliteV1
} // namespace Migration