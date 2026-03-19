-- ============================================================================
-- SCRIPT MIGRASI DATA: SKEMA LAMA → SKEMA BARU
-- ============================================================================
-- Script ini memindahkan data dari database lama ke database baru
-- PENTING: Backup database sebelum menjalankan script ini!
-- ============================================================================

-- Attach database lama
ATTACH DATABASE 'percetakan_old.db' AS old_db;

PRAGMA foreign_keys = OFF;
BEGIN TRANSACTION;

-- ============================================================================
-- 1. MIGRASI DATA MASTER
-- ============================================================================

-- Migrasi Roles (sudah ada di skema baru, skip)
-- Migrasi Price Levels (sudah ada di skema baru, cek apakah perlu update)

-- Migrasi Admins
INSERT INTO admins (
    id, role_id, username, password_hash, nama_lengkap, 
    is_active, last_login, created_at
)
SELECT 
    id, role_id, username, password_hash, nama_lengkap,
    is_active, last_login, created_at
FROM old_db.admins;

PRINT 'Admins migrated: ' || CHANGES();

-- Migrasi Konsumen
INSERT INTO konsumen (
    id, nama_lengkap, email, nomor_telp, alamat, 
    last_seen, created_at
)
SELECT 
    id, nama_lengkap, email, nomor_telp, alamat,
    last_seen, created_at
FROM old_db.konsumen;

PRINT 'Konsumen migrated: ' || CHANGES();

-- Update customer stats (akan dihitung dari orders)
UPDATE konsumen SET
    total_orders = (
        SELECT COUNT(*) 
        FROM old_db.orders 
        WHERE customer_id = konsumen.id
    ),
    total_spent = (
        SELECT COALESCE(SUM(total_amount), 0)
        FROM old_db.orders
        WHERE customer_id = konsumen.id 
        AND status != 'cancelled'
    );

PRINT 'Customer stats updated';

-- ============================================================================
-- 2. MIGRASI KATEGORI PRODUK
-- ============================================================================

-- Product categories sudah ada di skema baru dengan data default
-- Jika di database lama ada kategori custom, insert manual di sini

-- ============================================================================
-- 3. MIGRASI PRODUK
-- ============================================================================

INSERT INTO products (
    id, sku, name, stock, is_active
)
SELECT 
    id, sku, name, stock, is_active
FROM old_db.products;

PRINT 'Products migrated: ' || CHANGES();

-- Set category default untuk produk (bisa disesuaikan manual nanti)
UPDATE products SET category_id = 6 WHERE category_id IS NULL; -- Kategori "Lainnya"

-- ============================================================================
-- 4. MIGRASI HARGA PRODUK
-- ============================================================================

INSERT INTO product_prices (
    product_id, price_level_id, price, updated_at
)
SELECT 
    product_id, price_level_id, price, updated_at
FROM old_db.product_prices;

PRINT 'Product prices migrated: ' || CHANGES();

-- ============================================================================
-- 5. MIGRASI FINISHING SERVICES
-- ============================================================================

INSERT INTO finishing_services (
    id, name, price_per_unit
)
SELECT 
    id, name, price_per_unit
FROM old_db.finishing_services;

PRINT 'Finishing services migrated: ' || CHANGES();

-- ============================================================================
-- 6. MIGRASI ORDERS
-- ============================================================================

INSERT INTO orders (
    id, order_number, customer_id, customer_name, 
    total_amount, status, created_at, admin_id
)
SELECT 
    id, order_number, customer_id, customer_name,
    total_amount, 
    CASE 
        WHEN status = 'pending' THEN 'pending'
        WHEN status = 'processing' THEN 'processing'
        WHEN status = 'completed' THEN 'completed'
        ELSE status
    END as status,
    created_at, admin_id
FROM old_db.orders;

PRINT 'Orders migrated: ' || CHANGES();

-- Update payment status berdasarkan pembayaran
UPDATE orders SET
    paid_amount = (
        SELECT COALESCE(SUM(amount), 0)
        FROM old_db.payments
        WHERE order_id = orders.id
    ),
    payment_status = CASE
        WHEN (SELECT COALESCE(SUM(amount), 0) FROM old_db.payments WHERE order_id = orders.id) >= total_amount 
        THEN 'paid'
        WHEN (SELECT COALESCE(SUM(amount), 0) FROM old_db.payments WHERE order_id = orders.id) > 0 
        THEN 'partial'
        ELSE 'unpaid'
    END;

PRINT 'Order payment status updated';

-- Copy customer phone untuk denormalisasi
UPDATE orders SET
    customer_phone = (
        SELECT nomor_telp 
        FROM konsumen 
        WHERE konsumen.id = orders.customer_id
    )
WHERE customer_id IS NOT NULL;

-- Set order_date sama dengan created_at untuk data lama
UPDATE orders SET order_date = created_at WHERE order_date IS NULL;

-- ============================================================================
-- 7. MIGRASI ORDER ITEMS
-- ============================================================================

INSERT INTO order_items (
    id, order_id, product_id, quantity, base_price, subtotal
)
SELECT 
    id, order_id, product_id, quantity, base_price, subtotal
FROM old_db.order_items;

PRINT 'Order items migrated: ' || CHANGES();

-- Update denormalisasi product info
UPDATE order_items SET
    product_name = (SELECT name FROM products WHERE id = order_items.product_id),
    sku = (SELECT sku FROM products WHERE id = order_items.product_id),
    unit = (SELECT unit FROM products WHERE id = order_items.product_id)
WHERE product_id IS NOT NULL;

-- ============================================================================
-- 8. MIGRASI ORDER ITEM FINISHINGS
-- ============================================================================

INSERT INTO order_item_finishings (
    id, order_item_id, finishing_id, finishing_price
)
SELECT 
    id, order_item_id, finishing_id, finishing_price
FROM old_db.order_item_finishings;

PRINT 'Order item finishings migrated: ' || CHANGES();

-- Update denormalisasi finishing info
UPDATE order_item_finishings SET
    finishing_name = (SELECT name FROM finishing_services WHERE id = order_item_finishings.finishing_id),
    subtotal = finishing_price  -- Jika quantity selalu 1
WHERE finishing_id IS NOT NULL;

-- ============================================================================
-- 9. MIGRASI KATEGORI TRANSAKSI
-- ============================================================================

-- Kategori transaksi sudah ada di skema baru
-- Jika ada kategori custom, perlu disesuaikan mapping-nya

INSERT OR IGNORE INTO kategori_transaksi (id, nama, tipe)
SELECT id, nama, tipe
FROM old_db.kategori_transaksi;

PRINT 'Kategori transaksi migrated: ' || CHANGES();

-- ============================================================================
-- 10. MIGRASI TRANSAKSI
-- ============================================================================

INSERT INTO transaksi (
    id, admin_id, kategori_id, deskripsi, jumlah, tanggal
)
SELECT 
    id, admin_id, kategori_id, deskripsi, 
    CAST(jumlah AS REAL),  -- Convert INTEGER ke REAL
    tanggal
FROM old_db.transaksi;

PRINT 'Transaksi migrated: ' || CHANGES();

-- Update tipe transaksi dari kategori
UPDATE transaksi SET
    tipe = (
        SELECT tipe 
        FROM kategori_transaksi 
        WHERE kategori_transaksi.id = transaksi.kategori_id
    )
WHERE kategori_id IS NOT NULL;

-- ============================================================================
-- 11. MIGRASI PAYMENTS
-- ============================================================================

INSERT INTO payments (
    id, order_id, amount, payment_method, 
    transfer_account_number, transfer_verified,
    payment_date, admin_id
)
SELECT 
    id, order_id, amount, 
    CASE 
        WHEN method = 'cash' THEN 'cash'
        WHEN method = 'transfer' THEN 'transfer'
        ELSE 'cash'
    END as payment_method,
    transfer_acc, transfer_ver,
    paytime, admin_id
FROM old_db.payments;

PRINT 'Payments migrated: ' || CHANGES();

-- Update customer_id di payments
UPDATE payments SET
    customer_id = (
        SELECT customer_id 
        FROM orders 
        WHERE orders.id = payments.order_id
    );

-- Set payment_status berdasarkan transfer_verified
UPDATE payments SET
    payment_status = CASE
        WHEN payment_method = 'cash' THEN 'verified'
        WHEN transfer_verified = 1 THEN 'verified'
        ELSE 'pending'
    END;

-- ============================================================================
-- 12. GENERATE STOCK MOVEMENTS (OPTIONAL)
-- ============================================================================

-- Buat stock movement history untuk order yang sudah ada
-- Ini opsional, bisa di-skip jika tidak perlu history

INSERT INTO stock_movements (
    product_id, movement_type, quantity, 
    stock_before, stock_after,
    reference_type, reference_id,
    notes, admin_id, movement_date
)
SELECT 
    oi.product_id,
    'out',
    -oi.quantity,
    p.stock + oi.quantity,
    p.stock,
    'order',
    oi.order_id,
    'Historical data from migration - Order #' || o.order_number,
    o.admin_id,
    o.created_at
FROM old_db.order_items oi
JOIN old_db.orders o ON oi.order_id = o.id
JOIN products p ON oi.product_id = p.id
WHERE oi.product_id IS NOT NULL 
AND o.status != 'cancelled';

PRINT 'Stock movements created: ' || CHANGES();

-- ============================================================================
-- 13. AUTO-INCREMENT RESET
-- ============================================================================

-- Reset auto-increment untuk semua tabel ke nilai tertinggi + 1
-- Ini penting agar ID baru tidak konflik

UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM admins) WHERE name = 'admins';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM konsumen) WHERE name = 'konsumen';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM products) WHERE name = 'products';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM product_categories) WHERE name = 'product_categories';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM finishing_services) WHERE name = 'finishing_services';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM orders) WHERE name = 'orders';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM order_items) WHERE name = 'order_items';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM order_item_finishings) WHERE name = 'order_item_finishings';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM payments) WHERE name = 'payments';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM transaksi) WHERE name = 'transaksi';
UPDATE sqlite_sequence SET seq = (SELECT MAX(id) FROM kategori_transaksi) WHERE name = 'kategori_transaksi';

PRINT 'Auto-increment sequences updated';

-- ============================================================================
-- 14. VERIFIKASI DATA
-- ============================================================================

-- Hitung total records yang dimigrasikan
SELECT 'MIGRATION SUMMARY:';
SELECT '==================';
SELECT 'Admins: ' || COUNT(*) FROM admins;
SELECT 'Konsumen: ' || COUNT(*) FROM konsumen;
SELECT 'Products: ' || COUNT(*) FROM products;
SELECT 'Product Prices: ' || COUNT(*) FROM product_prices;
SELECT 'Finishing Services: ' || COUNT(*) FROM finishing_services;
SELECT 'Orders: ' || COUNT(*) FROM orders;
SELECT 'Order Items: ' || COUNT(*) FROM order_items;
SELECT 'Order Item Finishings: ' || COUNT(*) FROM order_item_finishings;
SELECT 'Payments: ' || COUNT(*) FROM payments;
SELECT 'Transaksi: ' || COUNT(*) FROM transaksi;
SELECT '==================';

-- Cek data yang mungkin bermasalah
SELECT 'POTENTIAL ISSUES:';
SELECT '==================';

-- Orders tanpa customer
SELECT 'Orders without customer: ' || COUNT(*) 
FROM orders WHERE customer_id IS NULL;

-- Order items tanpa product
SELECT 'Order items without product: ' || COUNT(*) 
FROM order_items WHERE product_id IS NULL;

-- Payments dengan amount 0
SELECT 'Payments with zero amount: ' || COUNT(*) 
FROM payments WHERE amount = 0;

-- Orders dengan total_amount != sum(order_items.subtotal)
SELECT 'Orders with inconsistent totals: ' || COUNT(*)
FROM orders o
WHERE ABS(o.total_amount - (
    SELECT COALESCE(SUM(subtotal), 0) 
    FROM order_items 
    WHERE order_id = o.id
)) > 0.01;

SELECT '==================';

-- ============================================================================
-- SELESAI
-- ============================================================================

COMMIT;
PRAGMA foreign_keys = ON;

-- Detach database lama
DETACH DATABASE old_db;

-- ============================================================================
-- POST-MIGRATION TASKS
-- ============================================================================

-- 1. VACUUM database untuk optimize
VACUUM;

-- 2. ANALYZE untuk update query planner statistics
ANALYZE;

-- ============================================================================
-- CATATAN PENTING
-- ============================================================================
-- 
-- Setelah migrasi selesai:
-- 
-- 1. CEK hasil verifikasi di atas
-- 2. TEST aplikasi dengan database baru
-- 3. CEK laporan-laporan masih akurat
-- 4. BACKUP database baru
-- 5. Simpan database lama sebagai backup
-- 6. Update connection string di aplikasi
-- 
-- Jika ada masalah:
-- - Rollback ke database lama
-- - Perbaiki script migrasi
-- - Ulangi proses
-- 
-- ============================================================================