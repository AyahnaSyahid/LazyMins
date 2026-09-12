CREATE TABLE roles (
    id INTEGER PRIMARY KEY,
    role_name TEXT NOT NULL UNIQUE,  -- Nama role: super_admin, kasir, operator
    description TEXT,                -- Deskripsi role
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

---- SEP

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

---- SEP

CREATE INDEX idx_admins_username ON admins(username);

---- SEP

CREATE INDEX idx_admins_role ON admins(role_id);

---- SEP

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

---- SEP

CREATE INDEX idx_konsumen_nama ON konsumen(nama_lengkap);

---- SEP

CREATE INDEX idx_konsumen_telp ON konsumen(nomor_telp);

---- SEP

CREATE INDEX idx_konsumen_code ON konsumen(customer_code);

---- SEP

CREATE TABLE product_categories (
    id INTEGER PRIMARY KEY,
    category_name TEXT NOT NULL UNIQUE,
    description TEXT,
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

---- SEP

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

---- SEP

CREATE UNIQUE INDEX idx_products_sku ON products(sku COLLATE NOCASE);

---- SEP

CREATE INDEX idx_products_name ON products(name);

---- SEP

CREATE INDEX idx_products_category ON products(category_id);

---- SEP

CREATE TABLE price_levels (
    id INTEGER PRIMARY KEY,
    level_name TEXT NOT NULL UNIQUE,
    discount_percentage REAL DEFAULT 0,  -- TAMBAHAN: Persentase diskon
    description TEXT,                     -- TAMBAHAN: Deskripsi level
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

---- SEP

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

---- SEP

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

---- SEP

CREATE INDEX idx_finishing_name ON finishing_services(name);

---- SEP

CREATE TABLE orders (
    id                  INTEGER PRIMARY KEY,
    order_number        TEXT UNIQUE,                    -- Di-generate otomatis
    invoice_id          INTEGER,                        -- Referensi ke invoice (bisa NULL jika belum dibuat)
    invoice_number      TEXT,                           -- Denormalisasi untuk tampilan cepat
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

---- SEP

CREATE INDEX idx_orders_invoice ON orders(invoice_id);

---- SEP

CREATE INDEX idx_orders_customer ON orders(customer_id);

---- SEP

CREATE INDEX idx_orders_status ON orders(staging_status);

---- SEP

CREATE INDEX idx_orders_deadline ON orders(deadline_date);

---- SEP

CREATE INDEX idx_orders_creation ON orders(created_at);

---- SEP

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

---- SEP

CREATE INDEX idx_order_items_order ON order_items(order_id);

---- SEP

CREATE INDEX idx_order_items_product ON order_items(product_id);

---- SEP

CREATE INDEX idx_order_items_creation ON order_items(created_at);

---- SEP

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

---- SEP

CREATE INDEX idx_order_finishings_item ON order_item_finishings(order_item_id);

---- SEP

CREATE TABLE payments (
    id                       INTEGER PRIMARY KEY,
    payment_number           TEXT UNIQUE,               -- PAY-20260320-00001
    invoice_id               INTEGER NOT NULL,          -- Referensi UTAMA (wajib)
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
    FOREIGN KEY (akun_transaksi_id) REFERENCES akun_transaksi(id)  ON DELETE RESTRICT
);

---- SEP

CREATE INDEX idx_payments_admin   ON payments(admin_id);

---- SEP

CREATE INDEX idx_payments_invoice ON payments(invoice_id);

---- SEP

CREATE INDEX idx_payments_date    ON payments(payment_date);

---- SEP

CREATE INDEX idx_payments_status  ON payments(verification_status);

---- SEP

CREATE INDEX idx_payments_akun_tr ON payments(akun_transaksi_id);

---- SEP

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

---- SEP

CREATE TABLE transaksi (
    id INTEGER PRIMARY KEY,
    akun_id INTEGER NOT NULL,
    transaction_number TEXT UNIQUE,   -- TAMBAHAN: Nomor transaksi unik
    admin_id INTEGER NOT NULL,
    kategori_id INTEGER,
    tipe TEXT NOT NULL COLLATE NOCASE CHECK( tipe IN ('pemasukan', 'pengeluaran', 'opname')),
    deskripsi TEXT,
    amount_before INT NOT NULL,
    amount INT NOT NULL,
    amount_after INT NOT NULL,
    payment_method TEXT,              -- TAMBAHAN: Metode pembayaran
    reference_type TEXT,              -- TAMBAHAN: order, payment, expense
    reference_id INTEGER,             -- TAMBAHAN: ID referensi (order_id, payment_id, dll)
    attachment TEXT,                  -- TAMBAHAN: Path file lampiran (nota, bukti)
    tanggal DATE NOT NULL DEFAULT (DATE('now')),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (akun_id) REFERENCES akun_transaksi(id) ON DELETE RESTRICT,
    FOREIGN KEY (admin_id) REFERENCES admins(id) ON DELETE RESTRICT,
    FOREIGN KEY (kategori_id) REFERENCES kategori_transaksi(id) ON DELETE RESTRICT
);

---- SEP

CREATE INDEX idx_transaksi_admin_tanggal ON transaksi(tanggal, admin_id);

---- SEP

CREATE INDEX idx_transaksi_kategori ON transaksi(kategori_id);

---- SEP

CREATE INDEX idx_transaksi_tipe ON transaksi(tipe);

---- SEP

CREATE INDEX idx_transaksi_akun ON transaksi(akun_id);

---- SEP

CREATE INDEX idx_transaksi_tanggal ON transaksi(tanggal);

---- SEP

CREATE INDEX idx_transaksi_creation ON transaksi(created_at);

---- SEP

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
    nama_bank TEXT,                       -- 'BRI', 'BCA', 'Mandiri', dll
    nomor_rekening TEXT,                  -- Nomor rekening/akun
    atas_nama TEXT,                       -- Nama pemilik rekening
    saldo INTEGER DEFAULT 0,
    is_active INTEGER DEFAULT 1,
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

---- SEP

CREATE TABLE stock_movements (
    id INTEGER PRIMARY KEY,
    product_id INTEGER NOT NULL,
    movement_type TEXT NOT NULL COLLATE NOCASE CHECK(movement_type IN ('in', 'out', 'adjustment')),
    stock_before REAL NOT NULL,       -- Stok sebelum transaksi
    quantity     REAL NOT NULL,       -- Positif untuk masuk, negatif untuk keluar
    stock_after  REAL NOT NULL,       -- Stok setelah transaksi
    reference_type TEXT,              -- order, purchase, adjustment
    reference_id INTEGER,             -- ID referensi (order_id, dll)
    notes TEXT,
    admin_id INTEGER NOT NULL,
    movement_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT,
    FOREIGN KEY (admin_id) REFERENCES admins(id)
);

---- SEP

CREATE INDEX idx_stock_movements_product ON stock_movements(product_id);

---- SEP

CREATE INDEX idx_stock_product_id ON stock_movements(product_id, id);

---- SEP

CREATE INDEX idx_stock_movements_date ON stock_movements(movement_date);

---- SEP

CREATE INDEX idx_stock_movements_type ON stock_movements(movement_type);

---- SEP

CREATE INDEX idx_stock_movements_creation ON stock_movements(created_at);

---- SEP

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

---- SEP

CREATE INDEX idx_activity_logs_admin ON activity_logs(admin_id);

---- SEP

CREATE INDEX idx_activity_logs_date ON activity_logs(created_at);

---- SEP

CREATE INDEX idx_activity_logs_action ON activity_logs(action);

---- SEP

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

---- SEP

CREATE TABLE invoices (
    id               INTEGER    PRIMARY KEY AUTOINCREMENT,
    invoice_number   TEXT       UNIQUE NOT NULL,
    customer_id      INTEGER,   -- sengaja untuk konsumen yang tidak terdaftar
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
    parent_id        INTEGER,   -- Referensi ke ID invoice sebelum direvisi
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

---- SEP

CREATE TABLE sqlite_sequence(name,seq);

---- SEP

CREATE INDEX idx_invoices_number ON invoices(invoice_number);

---- SEP

CREATE INDEX idx_invoices_customer ON invoices(customer_id);

---- SEP

CREATE INDEX idx_invoices_due_date ON invoices(due_date);

---- SEP

CREATE INDEX idx_invoices_status ON invoices(staging_status);

---- SEP

CREATE INDEX idx_invoices_creation ON invoices(created_at);

---- SEP

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

---- SEP

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

---- SEP

CREATE VIEW v_daily_sales AS
SELECT 
    DATE(o.order_date) AS sale_date,
    COUNT(DISTINCT o.id) AS total_orders,
    SUM(o.total_amount) AS total_sales,
    COUNT(DISTINCT o.customer_id) AS unique_customers
FROM orders o
WHERE o.staging_status != 'cancelled'
GROUP BY DATE(o.order_date);

---- SEP

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

---- SEP

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
