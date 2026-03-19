-- ============================================================================
-- SKEMA DATABASE APLIKASI ADMINISTRASI PERCETAKAN (IMPROVED VERSION)
-- ============================================================================
-- Versi yang disempurnakan dengan fitur tambahan untuk manajemen order,
-- pembayaran, inventori, dan laporan keuangan percetakan
-- ============================================================================

PRAGMA foreign_keys = ON;
BEGIN TRANSACTION;

-- ============================================================================
-- 1. TABEL MASTER - MANAJEMEN PENGGUNA & ROLES
-- ============================================================================

-- Tabel Roles: Mendefinisikan peran pengguna dalam sistem
CREATE TABLE roles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    role_name TEXT NOT NULL UNIQUE,  -- Nama role: super_admin, kasir, operator
    description TEXT,                 -- Deskripsi role
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);


-- Tabel Admins: Pengguna/staff yang mengoperasikan sistem
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

-- Index untuk mempercepat pencarian admin
CREATE INDEX idx_admins_username ON admins(username);
CREATE INDEX idx_admins_role ON admins(role_id);

-- ============================================================================
-- 2. TABEL MASTER - MANAJEMEN PELANGGAN
-- ============================================================================

-- Tabel Konsumen: Data pelanggan (diperbaiki dengan field tambahan)
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

-- Index untuk mempercepat pencarian konsumen
CREATE INDEX idx_konsumen_nama ON konsumen(nama_lengkap);
CREATE INDEX idx_konsumen_telp ON konsumen(nomor_telp);
CREATE INDEX idx_konsumen_code ON konsumen(customer_code);

-- ============================================================================
-- 3. TABEL MASTER - PRODUK & HARGA
-- ============================================================================

-- Tabel Kategori Produk (TAMBAHAN BARU)
CREATE TABLE product_categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    category_name TEXT NOT NULL UNIQUE,
    description TEXT,
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabel Products: Produk yang dijual (diperbaiki)
CREATE TABLE products (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sku TEXT UNIQUE NOT NULL,         -- Stock Keeping Unit
    name TEXT NOT NULL,
    category_id INTEGER,              -- TAMBAHAN: Kategori produk
    description TEXT,                 -- TAMBAHAN: Deskripsi produk
    unit TEXT DEFAULT 'pcs',          -- TAMBAHAN: Satuan (pcs, lembar, meter, dll)
    stock INTEGER DEFAULT 0,
    min_stock INTEGER DEFAULT 0,      -- TAMBAHAN: Minimum stok untuk alert
    cost_price REAL DEFAULT 0,        -- TAMBAHAN: Harga pokok (HPP)
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
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    level_name TEXT NOT NULL UNIQUE,
    discount_percentage REAL DEFAULT 0,  -- TAMBAHAN: Persentase diskon
    description TEXT,                     -- TAMBAHAN: Deskripsi level
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);



-- Tabel Product Prices: Harga produk berdasarkan level
CREATE TABLE product_prices (
    product_id INTEGER,
    price_level_id INTEGER,
    price REAL NOT NULL CHECK(price >= 0),
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
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    code TEXT UNIQUE,                 -- TAMBAHAN: Kode finishing (FIN-001)
    name TEXT NOT NULL,
    description TEXT,                 -- TAMBAHAN: Deskripsi layanan
    price_per_unit REAL DEFAULT 0,
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
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    order_number TEXT UNIQUE NOT NULL,
    customer_id INTEGER,
    customer_name TEXT NOT NULL,      -- Denormalisasi untuk performa
    customer_phone TEXT,              -- TAMBAHAN: Nomor telp customer
    price_level_id INTEGER DEFAULT 1, -- TAMBAHAN: Level harga yang digunakan
    
    -- Informasi finansial
    subtotal REAL DEFAULT 0,          -- TAMBAHAN: Subtotal sebelum diskon
    discount_amount REAL DEFAULT 0,   -- TAMBAHAN: Jumlah diskon
    discount_percentage REAL DEFAULT 0, -- TAMBAHAN: Persentase diskon
    tax_amount REAL DEFAULT 0,        -- TAMBAHAN: Jumlah pajak (PPN)
    total_amount REAL DEFAULT 0,      -- Total akhir
    
    -- Status dan tracking
    status TEXT DEFAULT 'pending',    -- pending, processing, ready, completed, cancelled
    priority TEXT DEFAULT 'normal',   -- TAMBAHAN: urgent, high, normal, low
    
    -- Jadwal
    order_date DATETIME DEFAULT CURRENT_TIMESTAMP,  -- TAMBAHAN: Tanggal order
    deadline_date DATETIME,           -- TAMBAHAN: Deadline pengerjaan
    completion_date DATETIME,         -- TAMBAHAN: Tanggal selesai
    
    -- Pembayaran
    payment_status TEXT DEFAULT 'unpaid',  -- TAMBAHAN: unpaid, partial, paid
    paid_amount REAL DEFAULT 0,       -- TAMBAHAN: Jumlah yang sudah dibayar
    
    -- Catatan
    notes TEXT,                       -- TAMBAHAN: Catatan order
    internal_notes TEXT,              -- TAMBAHAN: Catatan internal (tidak terlihat customer)
    
    -- Tracking
    admin_id INTEGER NOT NULL,        -- Admin yang membuat order
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (customer_id) REFERENCES konsumen(id) ON DELETE RESTRICT,
    FOREIGN KEY (admin_id) REFERENCES admins(id) ON DELETE RESTRICT,
    FOREIGN KEY (price_level_id) REFERENCES price_levels(id)
);

-- Index untuk orders
CREATE INDEX idx_orders_number ON orders(order_number);
CREATE INDEX idx_orders_customer ON orders(customer_id);
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_orders_date ON orders(order_date);
CREATE INDEX idx_orders_deadline ON orders(deadline_date);
CREATE INDEX idx_orders_payment_status ON orders(payment_status);

-- Tabel Order Items: Detail item dalam order (diperbaiki)
CREATE TABLE order_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    order_id INTEGER NOT NULL,
    product_id INTEGER,
    product_name TEXT NOT NULL,       -- TAMBAHAN: Denormalisasi nama produk
    sku TEXT,                         -- TAMBAHAN: Denormalisasi SKU
    quantity INTEGER NOT NULL CHECK(quantity > 0),
    unit TEXT DEFAULT 'pcs',          -- TAMBAHAN: Satuan
    base_price REAL NOT NULL,         -- Harga satuan
    discount_percentage REAL DEFAULT 0, -- TAMBAHAN: Diskon per item
    discount_amount REAL DEFAULT 0,   -- TAMBAHAN: Jumlah diskon
    subtotal REAL NOT NULL,           -- Total = (quantity * base_price) - discount + finishing
    notes TEXT,                       -- TAMBAHAN: Catatan khusus item
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT
);

-- Index untuk order items
CREATE INDEX idx_order_items_order ON order_items(order_id);
CREATE INDEX idx_order_items_product ON order_items(product_id);

-- Tabel Order Item Finishings: Finishing untuk setiap item order
CREATE TABLE order_item_finishings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    order_item_id INTEGER NOT NULL,
    finishing_id INTEGER,
    finishing_name TEXT NOT NULL,     -- TAMBAHAN: Denormalisasi nama finishing
    quantity INTEGER DEFAULT 1,       -- TAMBAHAN: Jumlah yang di-finishing
    finishing_price REAL NOT NULL,    -- Harga finishing per unit
    subtotal REAL NOT NULL,           -- TAMBAHAN: Total = quantity * finishing_price
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (order_item_id) REFERENCES order_items(id) ON DELETE CASCADE,
    FOREIGN KEY (finishing_id) REFERENCES finishing_services(id) ON DELETE RESTRICT
);

-- Index untuk order item finishings
CREATE INDEX idx_order_finishings_item ON order_item_finishings(order_item_id);

-- ============================================================================
-- 6. TABEL TRANSAKSI - PEMBAYARAN
-- ============================================================================

-- Tabel Payment Methods (TAMBAHAN BARU)
CREATE TABLE payment_methods (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    method_code TEXT UNIQUE NOT NULL,
    method_name TEXT NOT NULL,
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabel Payments: Pembayaran dari customer (diperbaiki)
CREATE TABLE payments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    payment_number TEXT UNIQUE,       -- TAMBAHAN: Nomor pembayaran unik (PAY-001)
    order_id INTEGER NOT NULL,
    customer_id INTEGER,              -- TAMBAHAN: Referensi ke customer
    
    -- Detail pembayaran
    amount REAL NOT NULL CHECK(amount > 0),
    payment_method TEXT NOT NULL DEFAULT 'cash',  -- cash, transfer, qris, dll
    
    -- Informasi transfer (jika method = transfer)
    transfer_bank TEXT,               -- TAMBAHAN: Nama bank
    transfer_account_name TEXT,       -- TAMBAHAN: Nama pemilik rekening
    transfer_account_number TEXT,     -- RENAME dari transfer_acc
    transfer_verified INTEGER DEFAULT 0,  -- RENAME dari transfer_ver
    transfer_proof_image TEXT,        -- TAMBAHAN: Path foto bukti transfer
    
    -- Informasi tunai (jika method = cash)
    cash_received REAL,               -- TAMBAHAN: Jumlah uang diterima
    cash_change REAL,                 -- TAMBAHAN: Kembalian
    
    -- Status dan tracking
    payment_status TEXT DEFAULT 'pending',  -- TAMBAHAN: pending, verified, cancelled
    notes TEXT,                       -- TAMBAHAN: Catatan pembayaran
    
    -- Tracking
    admin_id INTEGER NOT NULL,        -- Admin yang menerima pembayaran
    payment_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,  -- RENAME dari paytime
    verified_by INTEGER,              -- TAMBAHAN: Admin yang verifikasi
    verified_at DATETIME,             -- TAMBAHAN: Waktu verifikasi
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE RESTRICT,
    FOREIGN KEY (customer_id) REFERENCES konsumen(id),
    FOREIGN KEY (admin_id) REFERENCES admins(id) ON DELETE RESTRICT,
    FOREIGN KEY (verified_by) REFERENCES admins(id)
);

-- Index untuk payments
CREATE INDEX idx_payments_order ON payments(order_id);
CREATE INDEX idx_payments_customer ON payments(customer_id);
CREATE INDEX idx_payments_date ON payments(payment_date);
CREATE INDEX idx_payments_status ON payments(payment_status);
CREATE INDEX idx_payments_method ON payments(payment_method);

-- ============================================================================
-- 7. TABEL KEUANGAN - KATEGORI & TRANSAKSI
-- ============================================================================

-- Tabel Kategori Transaksi: Kategori pemasukan/pengeluaran (diperbaiki)
CREATE TABLE kategori_transaksi (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    kode TEXT UNIQUE,                 -- TAMBAHAN: Kode kategori (KAT-001)
    nama TEXT UNIQUE NOT NULL,
    tipe TEXT NOT NULL CHECK(tipe IN ('pemasukan', 'pengeluaran')),
    parent_id INTEGER,                -- TAMBAHAN: Untuk sub-kategori
    description TEXT,                 -- TAMBAHAN: Deskripsi kategori
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (parent_id) REFERENCES kategori_transaksi(id)
);

-- Tabel Transaksi: Catatan pemasukan/pengeluaran (diperbaiki)
CREATE TABLE transaksi (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    transaction_number TEXT UNIQUE,   -- TAMBAHAN: Nomor transaksi unik
    admin_id INTEGER NOT NULL,
    kategori_id INTEGER,
    
    -- Detail transaksi
    tipe TEXT NOT NULL CHECK(tipe IN ('pemasukan', 'pengeluaran')),
    deskripsi TEXT,
    jumlah REAL NOT NULL CHECK(jumlah > 0),
    
    -- Informasi tambahan
    payment_method TEXT,              -- TAMBAHAN: Metode pembayaran
    reference_type TEXT,              -- TAMBAHAN: order, payment, expense
    reference_id INTEGER,             -- TAMBAHAN: ID referensi (order_id, payment_id, dll)
    attachment TEXT,                  -- TAMBAHAN: Path file lampiran (nota, bukti)
    
    -- Tracking
    tanggal DATE NOT NULL DEFAULT (DATE('now')),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (admin_id) REFERENCES admins(id) ON DELETE RESTRICT,
    FOREIGN KEY (kategori_id) REFERENCES kategori_transaksi(id) ON DELETE RESTRICT
);

-- Index untuk transaksi
CREATE INDEX idx_transaksi_admin_tanggal ON transaksi(tanggal, admin_id);
CREATE INDEX idx_transaksi_kategori ON transaksi(kategori_id);
CREATE INDEX idx_transaksi_tipe ON transaksi(tipe);
CREATE INDEX idx_transaksi_tanggal ON transaksi(tanggal);

-- ============================================================================
-- 8. TABEL INVENTORI - STOCK MOVEMENTS (TAMBAHAN BARU)
-- ============================================================================

-- Tabel Stock Movements: Tracking pergerakan stok
CREATE TABLE stock_movements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id INTEGER NOT NULL,
    movement_type TEXT NOT NULL CHECK(movement_type IN ('in', 'out', 'adjustment')),
    quantity INTEGER NOT NULL,        -- Positif untuk masuk, negatif untuk keluar
    stock_before INTEGER NOT NULL,    -- Stok sebelum transaksi
    stock_after INTEGER NOT NULL,     -- Stok setelah transaksi
    
    -- Referensi
    reference_type TEXT,              -- order, purchase, adjustment
    reference_id INTEGER,             -- ID referensi (order_id, dll)
    
    -- Detail
    notes TEXT,
    admin_id INTEGER NOT NULL,
    movement_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT,
    FOREIGN KEY (admin_id) REFERENCES admins(id)
);

-- Index untuk stock movements
CREATE INDEX idx_stock_movements_product ON stock_movements(product_id);
CREATE INDEX idx_stock_movements_date ON stock_movements(movement_date);
CREATE INDEX idx_stock_movements_type ON stock_movements(movement_type);

-- ============================================================================
-- 9. TABEL AUDIT & LOG (TAMBAHAN BARU)
-- ============================================================================

-- Tabel Activity Logs: Log aktivitas user
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
    o.paid_amount,
    (o.total_amount - o.paid_amount) AS remaining_amount,
    o.status,
    o.payment_status,
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
    SUM(o.paid_amount) AS total_received,
    SUM(o.total_amount - o.paid_amount) AS total_outstanding,
    COUNT(DISTINCT o.customer_id) AS unique_customers
FROM orders o
WHERE o.status != 'cancelled'
GROUP BY DATE(o.order_date);

-- View: Top Selling Products
CREATE VIEW v_top_products AS
SELECT 
    p.id,
    p.sku,
    p.name,
    pc.category_name,
    COUNT(oi.id) AS order_count,
    SUM(oi.quantity) AS total_sold,
    SUM(oi.subtotal) AS total_revenue
FROM products p
LEFT JOIN product_categories pc ON p.category_id = pc.id
LEFT JOIN order_items oi ON p.id = oi.product_id
LEFT JOIN orders o ON oi.order_id = o.id
WHERE o.status != 'cancelled'
GROUP BY p.id
ORDER BY total_revenue DESC;

-- View: Customer Loyalty (Top Customers)
CREATE VIEW v_top_customers AS
SELECT 
    k.id,
    k.customer_code,
    k.nama_lengkap,
    k.nomor_telp,
    k.total_orders,
    k.total_spent,
    k.last_seen,
    CASE 
        WHEN k.total_spent >= 10000000 THEN 'VIP'
        WHEN k.total_spent >= 5000000 THEN 'Gold'
        WHEN k.total_spent >= 1000000 THEN 'Silver'
        ELSE 'Regular'
    END AS customer_tier
FROM konsumen k
WHERE k.is_active = 1
ORDER BY k.total_spent DESC;

-- ============================================================================
-- 12. TRIGGERS - AUTOMASI (TAMBAHAN BARU)
-- ============================================================================

-- Trigger: tambahkan product pada tabel stock_movement (tracking)

CREATE TRIGGER trg_add_product_stock
AFTER INSERT ON products
BEGIN
    INSERT INTO 
        stock_movement (
            product_id, movement_type, quantity,
            stock_before, stock_after, reference_type,
            reference_id, notes, admin_id, movement_date )
    VALUES ( NEW.id,      'in',  NEW.stock,
                  0, NEW.stock, 'products',
             NEW.id, 'Product stock init',
                  1, DATE('now'));
END;

-- Trigger: Update timestamp saat data berubah
CREATE TRIGGER trg_admins_updated_at 
AFTER UPDATE ON admins
BEGIN
    UPDATE admins SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER trg_konsumen_updated_at 
AFTER UPDATE ON konsumen
BEGIN
    UPDATE konsumen SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER trg_products_updated_at 
AFTER UPDATE ON products
BEGIN
    UPDATE products SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TRIGGER trg_orders_updated_at 
AFTER UPDATE ON orders
BEGIN
    UPDATE orders SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

-- Trigger: Auto-generate customer code
CREATE TRIGGER trg_konsumen_generate_code
AFTER INSERT ON konsumen
WHEN NEW.customer_code IS NULL
BEGIN
    UPDATE konsumen 
    SET customer_code = 'CUST-' || PRINTF('%05d', NEW.id)
    WHERE id = NEW.id;
END;

-- Trigger: Update stok saat order item ditambahkan (untuk produk stok)
CREATE TRIGGER trg_order_items_reduce_stock
AFTER INSERT ON order_items
WHEN NEW.product_id IS NOT NULL
BEGIN
    -- Kurangi stok produk
    UPDATE products 
    SET stock = stock - NEW.quantity 
    WHERE id = NEW.product_id;
    
    -- Catat stock movement
    INSERT INTO stock_movements (
        product_id, movement_type, quantity, 
        stock_before, stock_after, 
        reference_type, reference_id, 
        notes, admin_id
    )
    SELECT 
        NEW.product_id,
        'out',
        -NEW.quantity,
        p.stock + NEW.quantity,
        p.stock,
        'order',
        NEW.order_id,
        'Stock berkurang dari order #' || (SELECT order_number FROM orders WHERE id = NEW.order_id),
        (SELECT admin_id FROM orders WHERE id = NEW.order_id)
    FROM products p
    WHERE p.id = NEW.product_id;
END;

-- Trigger: Kembalikan stok saat order dibatalkan
CREATE TRIGGER trg_orders_cancel_restore_stock
AFTER UPDATE ON orders
WHEN NEW.status = 'cancelled' AND OLD.status != 'cancelled'
BEGIN
    -- Kembalikan stok untuk semua item di order
    UPDATE products 
    SET stock = stock + (
        SELECT SUM(quantity) 
        FROM order_items 
        WHERE order_id = NEW.id AND product_id = products.id
    )
    WHERE id IN (
        SELECT product_id 
        FROM order_items 
        WHERE order_id = NEW.id AND product_id IS NOT NULL
    );
END;

-- Trigger: Update payment status order saat ada pembayaran
CREATE TRIGGER trg_payments_update_order_status
AFTER INSERT ON payments
BEGIN
    UPDATE orders 
    SET 
        paid_amount = (
            SELECT COALESCE(SUM(amount), 0) 
            FROM payments 
            WHERE order_id = NEW.order_id 
            AND payment_status = 'verified'
        ),
        payment_status = CASE
            WHEN (SELECT COALESCE(SUM(amount), 0) FROM payments WHERE order_id = NEW.order_id AND payment_status = 'verified') >= total_amount 
            THEN 'paid'
            WHEN (SELECT COALESCE(SUM(amount), 0) FROM payments WHERE order_id = NEW.order_id AND payment_status = 'verified') > 0 
            THEN 'partial'
            ELSE 'unpaid'
        END
    WHERE id = NEW.order_id;
END;

-- Trigger: Update statistik customer
CREATE TRIGGER trg_orders_update_customer_stats
AFTER INSERT ON orders
BEGIN
    UPDATE konsumen 
    SET 
        total_orders = (SELECT COUNT(*) FROM orders WHERE customer_id = NEW.customer_id),
        total_spent = (SELECT COALESCE(SUM(total_amount), 0) FROM orders WHERE customer_id = NEW.customer_id AND status != 'cancelled'),
        last_seen = CURRENT_TIMESTAMP
    WHERE id = NEW.customer_id;
END;

-- ============================================================================
-- SELESAI
-- ============================================================================

-- Aktifkan kembali foreign keys
PRAGMA foreign_keys = ON;

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

-- DATA awal

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
('low_stock_alert', '10', 'number', 'Alert jika stok dibawah nilai ini');

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

-- Data awal kategori produk
INSERT INTO product_categories (category_name, description) VALUES
('Banner', 'Banner dan spanduk'),
('Brosur', 'Brosur dan flyer'),
('Kartu Nama', 'Business card dan kartu'),
('Stiker', 'Stiker dan label'),
('Fotocopy', 'Layanan fotocopy'),
('Lainnya', 'Produk lainnya');

-- Data awal price levels
INSERT INTO price_levels (id, level_name, discount_percentage, description) VALUES
(1, 'order', 0, 'Harga normal untuk pelanggan umum'),
(2, 'reseller', 10, 'Harga untuk reseller dengan diskon 10%'),
(3, 'wholesale', 15, 'Harga grosir dengan diskon 15%');

-- Data awal payment methods
INSERT INTO payment_methods (method_code, method_name) VALUES
('cash', 'Tunai'),
('transfer', 'Transfer Bank'),
('qris', 'QRIS'),
('debit', 'Kartu Debit'),
('credit', 'Kartu Kredit');

COMMIT;