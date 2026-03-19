-- ============================================================================
-- CONTOH QUERY APLIKASI PERCETAKAN
-- ============================================================================
-- Kumpulan query yang sering digunakan untuk operasional sehari-hari
-- ============================================================================

-- ============================================================================
-- 1. QUERY DASHBOARD
-- ============================================================================

-- Dashboard Ringkasan Hari Ini
SELECT 
    (SELECT COUNT(*) FROM orders WHERE DATE(order_date) = DATE('now')) AS orders_today,
    (SELECT COUNT(*) FROM orders WHERE DATE(order_date) = DATE('now') AND status = 'pending') AS pending_orders,
    (SELECT COUNT(*) FROM orders WHERE DATE(order_date) = DATE('now') AND status = 'processing') AS processing_orders,
    (SELECT COALESCE(SUM(total_amount), 0) FROM orders WHERE DATE(order_date) = DATE('now')) AS sales_today,
    (SELECT COALESCE(SUM(amount), 0) FROM payments WHERE DATE(payment_date) = DATE('now')) AS cash_received_today,
    (SELECT COUNT(*) FROM products WHERE stock <= min_stock AND is_active = 1) AS low_stock_products;

-- Pendapatan Bulan Ini
SELECT 
    COALESCE(SUM(total_amount), 0) AS total_sales,
    COALESCE(SUM(paid_amount), 0) AS total_received,
    COALESCE(SUM(total_amount - paid_amount), 0) AS total_outstanding
FROM orders 
WHERE strftime('%Y-%m', order_date) = strftime('%Y-%m', 'now')
AND status != 'cancelled';

-- Top 5 Produk Bulan Ini
SELECT 
    p.name,
    SUM(oi.quantity) AS total_sold,
    SUM(oi.subtotal) AS total_revenue
FROM order_items oi
JOIN products p ON oi.product_id = p.id
JOIN orders o ON oi.order_id = o.id
WHERE strftime('%Y-%m', o.order_date) = strftime('%Y-%m', 'now')
AND o.status != 'cancelled'
GROUP BY p.id, p.name
ORDER BY total_revenue DESC
LIMIT 5;

-- ============================================================================
-- 2. QUERY MANAJEMEN ORDER
-- ============================================================================

-- Daftar Order Pending (Belum Dikerjakan)
SELECT 
    o.id,
    o.order_number,
    o.customer_name,
    o.customer_phone,
    o.total_amount,
    o.payment_status,
    o.deadline_date,
    JULIANDAY(o.deadline_date) - JULIANDAY('now') AS days_remaining,
    a.nama_lengkap AS admin_name
FROM orders o
JOIN admins a ON o.admin_id = a.id
WHERE o.status = 'pending'
ORDER BY o.deadline_date ASC;

-- Daftar Order Urgent (Deadline < 2 Hari)
SELECT 
    o.id,
    o.order_number,
    o.customer_name,
    o.customer_phone,
    o.status,
    o.deadline_date,
    JULIANDAY(o.deadline_date) - JULIANDAY('now') AS days_remaining
FROM orders o
WHERE o.status NOT IN ('completed', 'cancelled')
AND JULIANDAY(o.deadline_date) - JULIANDAY('now') < 2
ORDER BY o.deadline_date ASC;

-- Detail Order Lengkap
SELECT 
    o.id,
    o.order_number,
    o.customer_name,
    o.customer_phone,
    o.status,
    o.payment_status,
    o.subtotal,
    o.discount_amount,
    o.tax_amount,
    o.total_amount,
    o.paid_amount,
    (o.total_amount - o.paid_amount) AS remaining,
    o.order_date,
    o.deadline_date,
    o.notes,
    a.nama_lengkap AS admin_name
FROM orders o
JOIN admins a ON o.admin_id = a.id
WHERE o.id = ?; -- Parameter: order_id

-- Item-item dalam Order
SELECT 
    oi.id,
    oi.product_name,
    oi.sku,
    oi.quantity,
    oi.unit,
    oi.base_price,
    oi.discount_amount,
    oi.subtotal,
    oi.notes
FROM order_items oi
WHERE oi.order_id = ?  -- Parameter: order_id
ORDER BY oi.id;

-- Finishing untuk Item Order
SELECT 
    oif.id,
    oif.finishing_name,
    oif.quantity,
    oif.finishing_price,
    oif.subtotal
FROM order_item_finishings oif
WHERE oif.order_item_id = ?  -- Parameter: order_item_id
ORDER BY oif.id;

-- Buat Order Baru (dengan customer baru)
BEGIN TRANSACTION;

-- Insert customer
INSERT INTO konsumen (nama_lengkap, nomor_telp, alamat, email)
VALUES (?, ?, ?, ?);  -- Parameters

-- Get customer_id
-- last_insert_rowid()

-- Insert order
INSERT INTO orders (
    order_number, customer_id, customer_name, customer_phone,
    subtotal, discount_amount, tax_amount, total_amount,
    status, priority, order_date, deadline_date,
    notes, admin_id
) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);

-- Get order_id
-- last_insert_rowid()

-- Insert order items
INSERT INTO order_items (
    order_id, product_id, product_name, sku,
    quantity, unit, base_price, discount_amount, subtotal
) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);

-- Insert finishing (optional)
INSERT INTO order_item_finishings (
    order_item_id, finishing_id, finishing_name,
    quantity, finishing_price, subtotal
) VALUES (?, ?, ?, ?, ?, ?);

COMMIT;

-- Update Status Order
UPDATE orders 
SET 
    status = ?,  -- pending/processing/ready/completed/cancelled
    updated_at = CURRENT_TIMESTAMP
WHERE id = ?;

-- ============================================================================
-- 3. QUERY PEMBAYARAN
-- ============================================================================

-- Daftar Order Belum Lunas
SELECT 
    o.id,
    o.order_number,
    o.customer_name,
    o.customer_phone,
    o.total_amount,
    o.paid_amount,
    (o.total_amount - o.paid_amount) AS remaining,
    o.payment_status,
    o.order_date
FROM orders o
WHERE o.payment_status IN ('unpaid', 'partial')
AND o.status != 'cancelled'
ORDER BY o.order_date ASC;

-- History Pembayaran untuk Order
SELECT 
    p.id,
    p.payment_number,
    p.amount,
    p.payment_method,
    p.payment_status,
    p.payment_date,
    p.notes,
    a.nama_lengkap AS received_by
FROM payments p
JOIN admins a ON p.admin_id = a.id
WHERE p.order_id = ?  -- Parameter: order_id
ORDER BY p.payment_date DESC;

-- Catat Pembayaran Tunai
INSERT INTO payments (
    payment_number, order_id, customer_id,
    amount, payment_method,
    cash_received, cash_change,
    payment_status, payment_date, admin_id
) VALUES (?, ?, ?, ?, 'cash', ?, ?, 'verified', CURRENT_TIMESTAMP, ?);

-- Catat Pembayaran Transfer
INSERT INTO payments (
    payment_number, order_id, customer_id,
    amount, payment_method,
    transfer_bank, transfer_account_name, transfer_account_number,
    transfer_proof_image, payment_status, payment_date, admin_id
) VALUES (?, ?, ?, ?, 'transfer', ?, ?, ?, ?, 'pending', CURRENT_TIMESTAMP, ?);

-- Verifikasi Pembayaran Transfer
UPDATE payments 
SET 
    payment_status = 'verified',
    verified_by = ?,
    verified_at = CURRENT_TIMESTAMP,
    updated_at = CURRENT_TIMESTAMP
WHERE id = ?;

-- Laporan Pembayaran Harian
SELECT 
    p.payment_method,
    COUNT(*) AS transaction_count,
    SUM(p.amount) AS total_amount
FROM payments p
WHERE DATE(p.payment_date) = DATE(?)  -- Parameter: tanggal
AND p.payment_status = 'verified'
GROUP BY p.payment_method;

-- ============================================================================
-- 4. QUERY MANAJEMEN PRODUK
-- ============================================================================

-- Daftar Produk Aktif dengan Harga
SELECT 
    p.id,
    p.sku,
    p.name,
    pc.category_name,
    p.stock,
    p.min_stock,
    p.unit,
    pp1.price AS price_order,
    pp2.price AS price_reseller
FROM products p
LEFT JOIN product_categories pc ON p.category_id = pc.id
LEFT JOIN product_prices pp1 ON p.id = pp1.product_id AND pp1.price_level_id = 1
LEFT JOIN product_prices pp2 ON p.id = pp2.product_id AND pp2.price_level_id = 2
WHERE p.is_active = 1
ORDER BY p.name;

-- Cari Produk
SELECT 
    p.id,
    p.sku,
    p.name,
    pc.category_name,
    p.stock,
    pp.price
FROM products p
LEFT JOIN product_categories pc ON p.category_id = pc.id
LEFT JOIN product_prices pp ON p.id = pp.product_id AND pp.price_level_id = 1
WHERE p.is_active = 1
AND (
    p.name LIKE '%' || ? || '%' 
    OR p.sku LIKE '%' || ? || '%'
)
ORDER BY p.name
LIMIT 20;

-- Update Stok Produk (Manual Adjustment)
BEGIN TRANSACTION;

-- Catat stock movement
INSERT INTO stock_movements (
    product_id, movement_type, quantity,
    stock_before, stock_after,
    reference_type, notes, admin_id
) VALUES (
    ?,  -- product_id
    'adjustment',
    ?,  -- quantity (positif/negatif)
    (SELECT stock FROM products WHERE id = ?),
    (SELECT stock FROM products WHERE id = ?) + ?,
    'adjustment',
    ?,  -- notes
    ?   -- admin_id
);

-- Update stok
UPDATE products 
SET 
    stock = stock + ?,  -- quantity
    updated_at = CURRENT_TIMESTAMP
WHERE id = ?;

COMMIT;

-- ============================================================================
-- 5. QUERY CUSTOMER
-- ============================================================================

-- Daftar Customer dengan Statistik
SELECT 
    k.id,
    k.customer_code,
    k.nama_lengkap,
    k.nomor_telp,
    k.email,
    k.total_orders,
    k.total_spent,
    k.last_seen,
    CASE 
        WHEN k.total_spent >= 10000000 THEN 'VIP'
        WHEN k.total_spent >= 5000000 THEN 'Gold'
        WHEN k.total_spent >= 1000000 THEN 'Silver'
        ELSE 'Regular'
    END AS tier
FROM konsumen k
WHERE k.is_active = 1
ORDER BY k.total_spent DESC;

-- Cari Customer
SELECT 
    k.id,
    k.customer_code,
    k.nama_lengkap,
    k.nomor_telp,
    k.email,
    k.total_orders,
    k.total_spent
FROM konsumen k
WHERE k.is_active = 1
AND (
    k.nama_lengkap LIKE '%' || ? || '%'
    OR k.nomor_telp LIKE '%' || ? || '%'
    OR k.customer_code LIKE '%' || ? || '%'
)
ORDER BY k.nama_lengkap
LIMIT 20;

-- History Order Customer
SELECT 
    o.id,
    o.order_number,
    o.total_amount,
    o.payment_status,
    o.status,
    o.order_date,
    o.deadline_date
FROM orders o
WHERE o.customer_id = ?
ORDER BY o.order_date DESC;

-- ============================================================================
-- 6. QUERY LAPORAN KEUANGAN
-- ============================================================================

-- Laporan Penjualan Periode
SELECT 
    DATE(o.order_date) AS tanggal,
    COUNT(*) AS total_order,
    SUM(o.subtotal) AS subtotal,
    SUM(o.discount_amount) AS total_diskon,
    SUM(o.tax_amount) AS total_pajak,
    SUM(o.total_amount) AS total_penjualan,
    SUM(o.paid_amount) AS total_terbayar,
    SUM(o.total_amount - o.paid_amount) AS total_piutang
FROM orders o
WHERE DATE(o.order_date) BETWEEN ? AND ?
AND o.status != 'cancelled'
GROUP BY DATE(o.order_date)
ORDER BY tanggal;

-- Laporan Pemasukan/Pengeluaran Periode
SELECT 
    DATE(t.tanggal) AS tanggal,
    SUM(CASE WHEN t.tipe = 'pemasukan' THEN t.jumlah ELSE 0 END) AS pemasukan,
    SUM(CASE WHEN t.tipe = 'pengeluaran' THEN t.jumlah ELSE 0 END) AS pengeluaran,
    SUM(CASE WHEN t.tipe = 'pemasukan' THEN t.jumlah ELSE -t.jumlah END) AS saldo
FROM transaksi t
WHERE DATE(t.tanggal) BETWEEN ? AND ?
GROUP BY DATE(t.tanggal)
ORDER BY tanggal;

-- Laporan per Kategori Transaksi
SELECT 
    kt.nama AS kategori,
    kt.tipe,
    COUNT(*) AS jumlah_transaksi,
    SUM(t.jumlah) AS total
FROM transaksi t
JOIN kategori_transaksi kt ON t.kategori_id = kt.id
WHERE DATE(t.tanggal) BETWEEN ? AND ?
GROUP BY kt.id, kt.nama, kt.tipe
ORDER BY kt.tipe, total DESC;

-- Profit Analysis (memerlukan cost_price di order_items)
SELECT 
    DATE(o.order_date) AS tanggal,
    SUM(oi.subtotal) AS revenue,
    SUM(oi.quantity * p.cost_price) AS cost,
    SUM(oi.subtotal - (oi.quantity * p.cost_price)) AS gross_profit,
    ROUND(
        (SUM(oi.subtotal - (oi.quantity * p.cost_price)) / SUM(oi.subtotal)) * 100, 
        2
    ) AS profit_margin_pct
FROM orders o
JOIN order_items oi ON o.id = oi.order_id
JOIN products p ON oi.product_id = p.id
WHERE DATE(o.order_date) BETWEEN ? AND ?
AND o.status != 'cancelled'
GROUP BY DATE(o.order_date)
ORDER BY tanggal;

-- ============================================================================
-- 7. QUERY INVENTORI
-- ============================================================================

-- Stock Movement History
SELECT 
    sm.id,
    sm.movement_date,
    sm.movement_type,
    p.name AS product_name,
    sm.quantity,
    sm.stock_before,
    sm.stock_after,
    sm.reference_type,
    sm.notes,
    a.nama_lengkap AS admin_name
FROM stock_movements sm
JOIN products p ON sm.product_id = p.id
LEFT JOIN admins a ON sm.admin_id = a.id
WHERE DATE(sm.movement_date) BETWEEN ? AND ?
ORDER BY sm.movement_date DESC;

-- Stock Opname Report
SELECT 
    p.id,
    p.sku,
    p.name,
    pc.category_name,
    p.stock AS system_stock,
    p.cost_price,
    p.stock * p.cost_price AS inventory_value
FROM products p
LEFT JOIN product_categories pc ON p.category_id = pc.id
WHERE p.is_active = 1
ORDER BY inventory_value DESC;

-- ============================================================================
-- 8. QUERY ADMIN & AUDIT
-- ============================================================================

-- Activity Log untuk Admin
SELECT 
    al.id,
    al.action,
    al.table_name,
    al.record_id,
    al.created_at,
    a.nama_lengkap AS admin_name
FROM activity_logs al
LEFT JOIN admins a ON al.admin_id = a.id
WHERE al.admin_id = ?
ORDER BY al.created_at DESC
LIMIT 50;

-- Performa Admin (Jumlah Order)
SELECT 
    a.id,
    a.nama_lengkap,
    r.role_name,
    COUNT(o.id) AS total_orders,
    SUM(o.total_amount) AS total_sales,
    COUNT(CASE WHEN o.status = 'completed' THEN 1 END) AS completed_orders
FROM admins a
LEFT JOIN roles r ON a.role_id = r.id
LEFT JOIN orders o ON a.id = o.admin_id 
    AND DATE(o.order_date) BETWEEN ? AND ?
WHERE a.is_active = 1
GROUP BY a.id, a.nama_lengkap, r.role_name
ORDER BY total_sales DESC;

-- ============================================================================
-- 9. QUERY UTILITY
-- ============================================================================

-- Generate Order Number
SELECT 
    COALESCE(
        (SELECT setting_value FROM app_settings WHERE setting_key = 'order_number_prefix'),
        'ORD'
    ) || '-' || 
    strftime('%Y%m%d', 'now') || '-' ||
    PRINTF('%04d', COALESCE(
        (SELECT COUNT(*) + 1 
         FROM orders 
         WHERE DATE(created_at) = DATE('now')),
        1
    )) AS next_order_number;

-- Generate Payment Number
SELECT 
    COALESCE(
        (SELECT setting_value FROM app_settings WHERE setting_key = 'payment_number_prefix'),
        'PAY'
    ) || '-' || 
    strftime('%Y%m%d', 'now') || '-' ||
    PRINTF('%04d', COALESCE(
        (SELECT COUNT(*) + 1 
         FROM payments 
         WHERE DATE(created_at) = DATE('now')),
        1
    )) AS next_payment_number;

-- Cek Integritas Data
SELECT 'Orders without customer' AS issue, COUNT(*) AS count
FROM orders WHERE customer_id IS NULL
UNION ALL
SELECT 'Order items without product', COUNT(*)
FROM order_items WHERE product_id IS NULL
UNION ALL
SELECT 'Negative stock products', COUNT(*)
FROM products WHERE stock < 0
UNION ALL
SELECT 'Orders with payment > total', COUNT(*)
FROM orders WHERE paid_amount > total_amount;

-- ============================================================================
-- 10. QUERY UNTUK EXPORT
-- ============================================================================

-- Export Semua Order ke CSV
SELECT 
    o.order_number AS 'No Order',
    o.order_date AS 'Tanggal',
    o.customer_name AS 'Customer',
    o.customer_phone AS 'Telepon',
    o.status AS 'Status',
    o.payment_status AS 'Status Bayar',
    o.total_amount AS 'Total',
    o.paid_amount AS 'Terbayar',
    a.nama_lengkap AS 'Admin'
FROM orders o
JOIN admins a ON o.admin_id = a.id
WHERE DATE(o.order_date) BETWEEN ? AND ?
ORDER BY o.order_date;

-- Export Transaksi Keuangan
SELECT 
    t.tanggal AS 'Tanggal',
    kt.nama AS 'Kategori',
    t.tipe AS 'Tipe',
    t.deskripsi AS 'Deskripsi',
    t.jumlah AS 'Jumlah',
    t.payment_method AS 'Metode',
    a.nama_lengkap AS 'Admin'
FROM transaksi t
JOIN kategori_transaksi kt ON t.kategori_id = kt.id
JOIN admins a ON t.admin_id = a.id
WHERE DATE(t.tanggal) BETWEEN ? AND ?
ORDER BY t.tanggal;

-- ============================================================================
-- CATATAN
-- ============================================================================
-- 
-- Tanda ? adalah parameter yang harus diisi saat query dijalankan
-- Sesuaikan query dengan kebutuhan aplikasi
-- Gunakan prepared statements untuk keamanan (prevent SQL injection)
-- Index sudah dibuat di skema untuk optimasi performa query ini
-- 
-- ============================================================================