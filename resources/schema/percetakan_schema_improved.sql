
-- di handle di aplikasi
-- PRAGMA foreign_keys = ON;
-- BEGIN TRANSACTION;

-- ============================================================================
-- 1. TABEL MASTER - MANAJEMEN PENGGUNA & ROLES
-- ============================================================================

-- Tabel Roles: Mendefinisikan peran pengguna dalam sistem
CREATE TABLE roles (
    id INTEGER PRIMARY KEY,
    role_name TEXT NOT NULL UNIQUE,  -- Nama role: super_admin, kasir, operator
    description TEXT,                -- Deskripsi role
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabel Admins: Pengguna/staff yang mengoperasikan sistem
CREATE TABLE admins (
    id INTEGER PRIMARY KEY,
    role_id INTEGER NOT NULL DEFAULT 3, -- Mulai sebagai Operator
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,        -- Password ter-hash (bcrypt/argon2)
    salt TEXT NOT NULL,                 -- Password ter-hash (bcrypt/argon2)
    nama_lengkap TEXT NOT NULL,
    email TEXT,                         -- TAMBAHAN: Email admin
    nomor_telp TEXT,                    -- TAMBAHAN: Nomor telepon admin
    is_active INTEGER DEFAULT 1,        -- 1: Aktif, 0: Non-aktif
    last_login DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE RESTRICT
);

-- Index untuk mempercepat pencarian admin
CREATE INDEX idx_admins_username ON admins(username);
CREATE INDEX idx_admins_role ON admins(role_id);

-- ============================================================================
-- 2. TABEL MASTER - MANAJEMEN PELANGGAN
-- ============================================================================

-- Tabel Konsumen: Data pelanggan (diperbaiki dengan field tambahan)
CREATE TABLE konsumen (
    id INTEGER PRIMARY KEY,
    customer_code TEXT UNIQUE,        -- TAMBAHAN: Kode unik pelanggan (CUST-001)
    nama_lengkap TEXT NOT NULL,
    customer_type TEXT DEFAULT 'Individual',  -- TAMBAHAN: individual/company
    email TEXT,                       -- Removed UNIQUE constraint (beberapa pelanggan bisa tidak punya email)
    nomor_telp TEXT,
    alamat TEXT,
    kota TEXT,                        -- TAMBAHAN: Kota pelanggan
    kode_pos TEXT,                    -- TAMBAHAN: Kode pos
    npwp TEXT,                        -- TAMBAHAN: NPWP untuk pelanggan korporat
    catatan TEXT,                     -- TAMBAHAN: Catatan khusus pelanggan
    price_level_id INTEGER DEFAULT 1, -- TAMBAHAN: Level harga default pelanggan
    is_active INTEGER DEFAULT 1,      -- TAMBAHAN: Status aktif/non-aktif
    last_seen DATETIME,               -- Terakhir kali order
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (price_level_id) REFERENCES price_levels(id)
);

-- Index untuk mempercepat pencarian konsumen
CREATE INDEX idx_konsumen_nama ON konsumen(nama_lengkap);
CREATE INDEX idx_konsumen_telp ON konsumen(nomor_telp);
CREATE INDEX idx_konsumen_code ON konsumen(customer_code);

-- ============================================================================
-- 3. TABEL MASTER - PRODUK & HARGA
-- ============================================================================

-- Tabel Kategori Produk (TAMBAHAN BARU)
CREATE TABLE product_categories (
    id INTEGER PRIMARY KEY,
    category_name TEXT NOT NULL UNIQUE,
    description TEXT,
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabel Products: Produk yang dijual (diperbaiki)
CREATE TABLE products (
    id INTEGER PRIMARY KEY,
    sku TEXT UNIQUE NOT NULL,         -- Stock Keeping Unit
    name TEXT NOT NULL,
    category_id INTEGER,              -- TAMBAHAN: Kategori produk
    description TEXT,                 -- TAMBAHAN: Deskripsi produk
    unit TEXT COLLATE NOCASE DEFAULT 'pcs',          -- TAMBAHAN: Satuan (pcs, lembar, meter, dll)
    stock REAL DEFAULT 0,
    min_stock REAL DEFAULT 0,         -- TAMBAHAN: Minimum stok untuk alert
    cost_price INTEGER DEFAULT 0,     -- TAMBAHAN: Harga pokok (HPP)
    use_area INTEGER DEFAULT 0,       -- TAMBAHAN: Apakah perhitungan harga berdasarkan Area
    is_active INTEGER DEFAULT 1,      -- 1: Aktif, 0: Non-aktif
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES product_categories(id)
);

-- Index untuk produk
CREATE UNIQUE INDEX idx_products_sku ON products(sku COLLATE NOCASE);
CREATE INDEX idx_products_name ON products(name);
CREATE INDEX idx_products_category ON products(category_id);

-- Tabel Price Levels: Level harga untuk berbagai tipe pelanggan
CREATE TABLE price_levels (
    id INTEGER PRIMARY KEY,
    level_name TEXT NOT NULL UNIQUE,
    discount_percentage REAL DEFAULT 0,  -- TAMBAHAN: Persentase diskon
    description TEXT,                     -- TAMBAHAN: Deskripsi level
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabel Product Prices: Harga produk berdasarkan level
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

-- ============================================================================
-- 4. TABEL MASTER - LAYANAN FINISHING
-- ============================================================================

-- Tabel Finishing Services: Layanan finishing tambahan (diperbaiki)
CREATE TABLE finishing_services (
    id INTEGER PRIMARY KEY,
    code TEXT UNIQUE,                 -- TAMBAHAN: Kode finishing (FIN-001)
    name TEXT NOT NULL,
    description TEXT,                 -- TAMBAHAN: Deskripsi layanan
    price_per_unit INTEGER DEFAULT 0,
    unit TEXT DEFAULT 'pcs',          -- TAMBAHAN: Satuan (pcs, lembar, meter)
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Index untuk finishing services
CREATE INDEX idx_finishing_name ON finishing_services(name);

-- ============================================================================
-- 5. TABEL TRANSAKSI - ORDERS
-- ============================================================================

-- Tabel Orders: Header order/pesanan (diperbaiki)
CREATE TABLE orders (
    id                  INTEGER PRIMARY KEY,
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

-- Index untuk orders
CREATE INDEX idx_orders_invoice ON orders(invoice_id);
CREATE INDEX idx_orders_customer ON orders(customer_id);
CREATE INDEX idx_orders_status ON orders(staging_status);
CREATE INDEX idx_orders_deadline ON orders(deadline_date);
CREATE INDEX idx_orders_creation ON orders(created_at);

-- Tabel Order Items: Detail item dalam order (diperbaiki)
CREATE TABLE order_items (
    id INTEGER PRIMARY KEY,
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
                    (CAST(((quantity * sale_price * size_width * size_height) + 99.99999) / 100 AS INT) * 100) + 
                        COALESCE(finishing_total, 0) - COALESCE(discount_amount,0) ) VIRTUAL,              -- Total =  Subtotal - ( discount + finishing )
    notes TEXT,                          -- TAMBAHAN: Catatan khusus item
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT
);

-- Index untuk order items
CREATE INDEX idx_order_items_order ON order_items(order_id);
CREATE INDEX idx_order_items_product ON order_items(product_id);
CREATE INDEX idx_order_items_creation ON order_items(created_at);

-- Tabel Order Item Finishings: Finishing untuk setiap item order
CREATE TABLE order_item_finishings (
    id INTEGER PRIMARY KEY,
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

-- Index untuk order item finishings
CREATE INDEX idx_order_finishings_item ON order_item_finishings(order_item_id);

-- ============================================================================
-- 6. TABEL TRANSAKSI - PEMBAYARAN
-- ============================================================================

-- Tabel Payments: Pembayaran dari customer (diperbaiki)
CREATE TABLE payments (
    id                       INTEGER PRIMARY KEY,
    payment_number           TEXT UNIQUE,               -- PAY-20260320-00001
    invoice_id               INTEGER NOT NULL,          -- Referensi UTAMA (wajib)

    -- Detail pembayaran
    amount                   INTEGER NOT NULL CHECK(amount > 0),
    akun_transaksi_id        INTEGER NOT NULL,
    
    -- Informasi tunai
    cash_received            INTEGER,
    cash_change              INTEGER,
    
    -- Status & catatan
    verification_status      TEXT DEFAULT 'pending' COLLATE NOCASE CHECK( verification_status IN ('pending','verified','cancelled') ),
    notes                    TEXT,
    
    -- Tracking
    admin_id                 INTEGER NOT NULL,
    payment_date             DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    verified_by              INTEGER,
    verified_at              DATETIME,

    created_at               DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at               DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (invoice_id)        REFERENCES invoices(id) ON DELETE RESTRICT,
    FOREIGN KEY (admin_id)          REFERENCES admins(id) ON DELETE RESTRICT,
    FOREIGN KEY (verified_by)       REFERENCES admins(id) ON DELETE RESTRICT,
    FOREIGN KEY (akun_transaksi_id) REFERENCES akun_transaksi(id)  ON DELETE RESTRICT
);

-- Index untuk payments
CREATE INDEX idx_payments_admin   ON payments(admin_id);
CREATE INDEX idx_payments_invoice ON payments(invoice_id);
CREATE INDEX idx_payments_date    ON payments(payment_date);
CREATE INDEX idx_payments_status  ON payments(verification_status);
CREATE INDEX idx_payments_akun_tr ON payments(akun_transaksi_id);


-- ============================================================================
-- 7. TABEL KEUANGAN - KATEGORI & TRANSAKSI
-- ============================================================================

-- Tabel Kategori Transaksi: Kategori pemasukan/pengeluaran (diperbaiki)
CREATE TABLE kategori_transaksi (
    id INTEGER PRIMARY KEY,
    kode TEXT UNIQUE,                 -- TAMBAHAN: Kode kategori (KAT-001)
    nama TEXT UNIQUE NOT NULL,
    tipe TEXT NOT NULL COLLATE NOCASE CHECK( tipe IN ('pemasukan', 'pengeluaran', 'opname')),
    parent_id INTEGER,                -- TAMBAHAN: Untuk sub-kategori
    description TEXT,                 -- TAMBAHAN: Deskripsi kategori
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (parent_id) REFERENCES kategori_transaksi(id)
);

-- Tabel Transaksi: Catatan pemasukan/pengeluaran (diperbaiki)
CREATE TABLE transaksi (
    id INTEGER PRIMARY KEY,
    akun_id INTEGER NOT NULL,
    transaction_number TEXT UNIQUE,   -- TAMBAHAN: Nomor transaksi unik
    admin_id INTEGER NOT NULL,
    kategori_id INTEGER,
    
    -- Detail transaksi
    tipe TEXT NOT NULL COLLATE NOCASE CHECK( tipe IN ('pemasukan', 'pengeluaran', 'opname')),
    deskripsi TEXT,
    
    -- Ledger Mode (Buku Besar)
    amount_before INT NOT NULL,
    amount INT NOT NULL,
    amount_after INT NOT NULL,
    
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

-- Index untuk transaksi
CREATE INDEX idx_transaksi_admin_tanggal ON transaksi(tanggal, admin_id);
CREATE INDEX idx_transaksi_kategori ON transaksi(kategori_id);
CREATE INDEX idx_transaksi_tipe ON transaksi(tipe);
CREATE INDEX idx_transaksi_akun ON transaksi(akun_id);
CREATE INDEX idx_transaksi_tanggal ON transaksi(tanggal);
CREATE INDEX idx_transaksi_creation ON transaksi(created_at);

-- Tabel Akun Transaksi: Rekening/Sumber Dana
CREATE TABLE akun_transaksi (
    id INTEGER PRIMARY KEY,
    kode TEXT UNIQUE NOT NULL,            -- Kode akun: 'CASH', 'BRI', 'BCA'
    nama TEXT UNIQUE NOT NULL,            -- Nama tampilan: 'Kas Admin', 'Bank BRI'
    tipe TEXT NOT NULL COLLATE NOCASE CHECK(tipe IN (
        'cash',         -- Uang tunai
        'bank',         -- Rekening bank
        'ewallet',      -- Dompet digital (OVO, DANA, GoPay)
        'lainnya'       -- Akun lain
    )),
    
    -- Informasi rekening (opsional, untuk bank/ewallet)
    nama_bank TEXT,                       -- 'BRI', 'BCA', 'Mandiri', dll
    nomor_rekening TEXT,                  -- Nomor rekening/akun
    atas_nama TEXT,                       -- Nama pemilik rekening
    
    -- Saldo
    saldo INTEGER DEFAULT 0,
    
    is_active INTEGER DEFAULT 1,
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 8.2 TABEL INVENTORI - STOCK MOVEMENTS (TAMBAHAN BARU)
-- ============================================================================

-- Tabel Stock Movements: Tracking pergerakan stok
CREATE TABLE stock_movements (
    id INTEGER PRIMARY KEY,
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

-- Index untuk stock movements
CREATE INDEX idx_stock_movements_product ON stock_movements(product_id);
CREATE INDEX idx_stock_product_id ON stock_movements(product_id, id);
CREATE INDEX idx_stock_movements_date ON stock_movements(movement_date);
CREATE INDEX idx_stock_movements_type ON stock_movements(movement_type);
CREATE INDEX idx_stock_movements_creation ON stock_movements(created_at);

-- ============================================================================
-- 9. TABEL AUDIT & LOG (TAMBAHAN BARU)
-- ============================================================================

-- Tabel Activity Logs: Log aktivitas user
CREATE TABLE activity_logs (
    id INTEGER PRIMARY KEY,
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

-- Index untuk activity logs
CREATE INDEX idx_activity_logs_admin ON activity_logs(admin_id);
CREATE INDEX idx_activity_logs_date ON activity_logs(created_at);
CREATE INDEX idx_activity_logs_action ON activity_logs(action);

-- ============================================================================
-- 10. TABEL SETTINGS & CONFIGURATION (TAMBAHAN BARU)
-- ============================================================================

-- Tabel App Settings: Pengaturan aplikasi
CREATE TABLE app_settings (
    id INTEGER PRIMARY KEY,
    setting_key TEXT UNIQUE NOT NULL,
    setting_value TEXT,
    data_type TEXT DEFAULT 'string',  -- string, number, boolean, json
    description TEXT,
    is_public INTEGER DEFAULT 0,      -- 0: hanya admin, 1: publik
    updated_by INTEGER,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (updated_by) REFERENCES admins(id)
);

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

    -- Logika Status yang Terpisah
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

-- Index penting
CREATE INDEX idx_invoices_number ON invoices(invoice_number);
CREATE INDEX idx_invoices_customer ON invoices(customer_id);
CREATE INDEX idx_invoices_due_date ON invoices(due_date);
CREATE INDEX idx_invoices_status ON invoices(staging_status);
CREATE INDEX idx_invoices_creation ON invoices(created_at);

-- ============================================================================
-- 11. VIEWS - UNTUK LAPORAN (TAMBAHAN BARU)
-- ============================================================================

-- View: Ringkasan Order
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

-- View: Produk dengan Stok Rendah
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

-- View: Laporan Penjualan Harian
CREATE VIEW v_daily_sales AS
SELECT 
    DATE(o.order_date) AS sale_date,
    COUNT(DISTINCT o.id) AS total_orders,
    SUM(o.total_amount) AS total_sales,
    COUNT(DISTINCT o.customer_id) AS unique_customers
FROM orders o
WHERE o.staging_status != 'cancelled'
GROUP BY DATE(o.order_date);

-- View: Top Selling Products
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

-- View: Customer Loyalty (Top Customers)
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
           LEFT JOIN
           orders o ON o.customer_id = k.id AND
                       o.staging_status != 'cancelled'-- jangan hitung order batal
     WHERE k.is_active = 1
     GROUP BY k.id,
              k.customer_code,
              k.nama_lengkap,
              k.nomor_telp,
              k.last_seen
     ORDER BY total_spent DESC;

-- ============================================================================
-- CATATAN PENGGUNAAN:
-- ============================================================================
-- 1. Jalankan script ini untuk membuat database baru
-- 2. Untuk migrasi dari database lama, buat script terpisah
-- 3. Backup database secara berkala
-- 4. Index sudah dioptimasi untuk query umum
-- 5. Triggers otomatis menangani update timestamp dan stok
-- 6. Views tersedia untuk laporan cepat
-- ============================================================================

-- ============================================================================
-- DATA PERCOBAAN
-- ============================================================================

-- Data awal settings
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

-- Finishing Services
INSERT INTO finishing_services (code, name, description, price_per_unit) VALUES 
('A3-DOF', 'L DOFF', 'A3Plus Laminasi Doff', 4000),
('A3-GLOS', 'L GLOS', 'A3Plus Laminasi Glossy', 4000),
('A3-CUT-SUMMA', 'CUT SUMMA', 'A3Plus Cutting - Summa', 5000),
('A3-CUT-SAGA', 'CUT SAGA', 'A3Plus Cutting - SAGA', 4000),
('A3-PTG-MIN', 'POTONG MIN', 'A3Plus Potong MIN', 4000),
('A3-PTG-MED', 'POTONG MED', 'A3Plus Potong MED', 8000),
('A3-PTG-HI', 'POTONG HIGH', 'A3Plus Potong HIGH', 12000);

-- Data awal roles
INSERT INTO roles (id, role_name, description) VALUES
(1, 'super_admin', 'Akses penuh ke seluruh sistem'),
(2, 'kasir', 'Menangani transaksi dan pembayaran'),
(3, 'operator', 'Mengelola order dan produksi');

-- Data awal kategori transaksi
INSERT INTO kategori_transaksi (nama, tipe, description) VALUES
('Penjualan Produk', 'pemasukan', 'Pemasukan dari penjualan produk'),
('Penjualan Jasa', 'pemasukan', 'Pemasukan dari jasa finishing'),
('Lain-lain (Pemasukan)', 'pemasukan', 'Pemasukan lainnya'),
('Pembelian Bahan', 'pengeluaran', 'Pengeluaran untuk bahan baku'),
('Gaji Karyawan', 'pengeluaran', 'Pengeluaran gaji'),
('Utilitas', 'pengeluaran', 'Listrik, air, internet'),
('Maintenance', 'pengeluaran', 'Perawatan mesin dan peralatan'),
('Lain-lain (Pengeluaran)', 'pengeluaran', 'Pengeluaran lainnya');

-- COMMENT INSERT INI Dalam PRODUKSI
-- INSERT INTO admins (id, role_id, username, password_hash, salt, nama_lengkap, email, nomor_telp, is_active, last_login, created_at, updated_at) VALUES 
-- (1, 1, 'nurholis', '7b918a1952245299d004b12501f1b8c8ece58f35dd0235eaa4f8bc08661f8eca', 'yt3ufKxhnhE5RCJm8a2ZqtFNDyLg3JSw', 'Nur Holis Komarudin', NULL, NULL, 1, NULL, '2026-02-24T16:49:59.097Z', '2026-02-24T16:49:59.097Z'),
-- (2, 2, 'maman', 'e44ed82d2c7b958b9373c9883dded54666d45cd2b8c3826eb9ed6c590196a9db', 'Scx6dhFGEKObSmKazZSAVXGkT0USTrR2', 'Maman Nurzaman', NULL, NULL, 1, NULL, '2026-02-24T16:49:59.190Z', '2026-02-24 16:49:59'),
-- (3, 3, 'syahid', '9ccca7d556bfc8ce14007a559094f62232c9d4edb2621b078728ab57b6c6e338', '5uLr1WBHUUc5cYV1bKiwsYJ7dChs50PZ', 'Syahid Yusuf Nurdiansyah', NULL, NULL, 1, NULL, '2026-02-24T16:49:59.300Z', '2026-02-24 16:49:59');

-- Data awal price levels
INSERT INTO price_levels (id, level_name, discount_percentage, description) VALUES
(1, 'ORDER', 0, 'Harga normal untuk pelanggan OD'),
(2, 'MAKLOON', 0, 'Harga normal untuk reseller MAKLOON'),
(3, 'NEGO', 0, 'Harga Nego BOS');

-- Data awal kategori produk
INSERT INTO product_categories (category_name, description) VALUES
('LargeFormat', 'Banner, Spanduk dan Large Format lainnya'),
('A3Plus',      'Brosur, Sticker, Flyer, dan Cetak menggunakan mesin A3 Plus'),
('CTP',         'Cetak CTP untuk percetakan offset'),
('LASER',       'Laser Cutting Akrilik, Kayu, MDF, dan lainnya'),
('Offset',      'Cetak Offset untuk Undangan, Kartu Nama dalam jumlah besar dan Cetak Offset lainnya');

-- updated_at untuk product_prices dikelola di level aplikasi (ProductPriceManager::upsert)
INSERT INTO products (sku, name, category_id, description, unit, stock, min_stock, cost_price, use_area) VALUES 
    ('BN-FLEX',    'FLEXY',           1, 'Cetak Banner Bahan Fleksi',           'meter',   120, 30, 15000, 1),
    ('BN-KOR',     'KOREA',           1, 'Cetak Banner Bahan Korea',            'meter',   80, 20,  45000, 1),
    ('STIND-KOR',  'Indoor KOREA',    1, 'Cetak Printer Indoor Bahan Korea',    'meter',   60, 15,  90000, 1),
    ('STIND-GRF',  'Indoor Graftack', 1, 'Cetak Printer Indoor Bahan Graftack', 'meter',   50, 10,  90000, 1),
    ('STIND-LUST', 'Indoor Luster',   1, 'Cetak Printer Indoor Bahan Luster',   'meter',   40, 10, 120000, 1),
    ('A3-AP150',   'AP150',           2, 'Cetak A3+ Bahan AP150',               'lembar',  200, 50,  2500, 0),
    ('A3-AP210',   'AP210',           2, 'Cetak A3+ Bahan AP210',               'lembar',  150, 40,  3000, 0),
    ('A3-AP230',   'AP230',           2, 'Cetak A3+ Bahan AP230',               'lembar',  100, 25,  3000, 0),
    ('A3-AP260',   'AP260',           2, 'Cetak A3+ Bahan AP260',               'lembar',  90, 20,   3000, 0),
    ('A3-AP260-BB','AP260 BB',        2, 'Cetak A3+ Bahan AP260 2Sisi',         'lembar',  80, 20,   5000, 0),
    ('A3-VNYL',    'VINYL',           2, 'Cetak A3+ Bahan VINYL',               'lembar',  70, 15,   8500, 0),
    ('A3-TRNS',    'TRANSPARENT',     2, 'Cetak A3+ Bahan TRANSPARENT',         'lembar',  60, 15,   8500, 0),
    ('A3-PVC',     'PVC',             2, 'Cetak A3+ Bahan PVC',                 'set',     50, 10,  75000, 0),
    ('A3-PVC-NF',  'PVCNF',           2, 'Cetak A3+ Bahan PVC Tanpa finishing', 'set',     40, 10,  60000, 0),
    ('A3-HVS',     'HVS',             2, 'Cetak A3+ Bahan HVS',                 'set',     200, 50,  2500, 0),
    ('A3-KALKIR',  'KALKIR',          2, 'Cetak A3+ Bahan KALKIR',              'set',     30, 10,  10000, 0),
    ('CTP-TOKO',   'Toko',            3, 'Pelat Toko',                          'set',     20, 5,   12000, 0),
    ('CTP-SORM',   'SORM',            3, 'Pelat SORM',                          'pcs',     15, 5,   20000, 0),
    ('CTP-P46',    'P46',             3, 'Pelat 46',                            'pcs',     10, 3,   15000, 0),
    ('CTP-P52',    'P52',             3, 'Pelat 52',                            'pcs',     8, 2,    35000, 0),
    ('OFF-TOKO',   'CO-TOKO',         5, 'Cetak Offset Toko',                   'set',     100, 25, 12000, 0),
    ('OFF-SORM-F', 'CO-SORM-F',       5, 'Cetak Offset SORM Full Color',        'set',     80, 20,  20000, 0),
    ('OFF-P46-F',  'CO-P46-F',        5, 'Cetak Offset P46 Full Color',         'set',     60, 15,  15000, 0),
    ('OFF-P52-F',  'CO-P52-F',        5, 'Cetak Offset P52 Full Color',         'set',     40, 10,  35000, 0);

-- buat beberapa test data konsumen setelah price levels dibuat, karena ada foreign key reference ke price_levels
INSERT INTO konsumen (customer_code, nama_lengkap, customer_type, email, nomor_telp, alamat, kota, kode_pos, npwp, catatan, price_level_id) VALUES
('CUST-001', 'PT. Sinar Jaya', 'Company', 'contact@sinarjaya.com', '021-12345678', 'Jl. Raya No. 123', 'Jakarta', '12345', '12.345.678.9-000.000', 'Pelanggan utama', 1),
('CUST-002', 'Budi Santoso', 'Individual', 'budi.santoso@email.com', '021-87654321', 'Jl. Merdeka No. 456', 'Bandung', '45678', NULL, 'Pelanggan baru', 1),
('CUST-003', 'CV. Maju Terus', 'Company', 'info@majuterus.com', '021-23456789', 'Jl. Pahlawan No. 789', 'Surabaya', '67890', '12.345.678.9-000.000', 'Pelanggan strategis', 1),
('CUST-004', 'Siti Aminah', 'Individual', NULL, '021-34567890', 'Jl. Sudirman No. 321', 'Medan', '54321', NULL, 'Pelanggan dengan potensi besar', 1),
('CUST-005', 'PT. Global Abadi', 'Company', 'info@globalabadi.com', '021-45678901', 'Jl. Diponegoro No. 567', 'Semarang', '78901', '12.345.678.9-000.000', 'Pelanggan utama', 1),
('CUST-006', 'Ahmad Fauzi', 'Individual', 'ahmad.fauzi@email.com', '021-56789012', 'Jl. Gatot Subroto No. 678', 'Yogyakarta', '89012', NULL, 'Pelanggan loyal', 1);

-- buat akun transaksi default
INSERT INTO akun_transaksi ( kode, nama, tipe, nama_bank, nomor_rekening, atas_nama, saldo, description ) VALUES 
( 'CASH', 'Kas Admin', 'cash', NULL, NULL, NULL, 0, 'Akun trasaksi default' )

-- buat inisiasi kas
-- INSERT INTO transaksi ( transaction_number, admin_id, kategori_id, tipe, deskripsi, amount_before, amount, amount_after) VALUES 
-- ('INIT', 1, 3, 'pemasukan', 'Inisialisasi Data Awal', 0, 0, 0);
-- ============================================================================
-- SELESAI
-- ============================================================================

-- COMMIT;

-- Aktifkan kembali foreign keys
-- PRAGMA foreign_keys = ON;