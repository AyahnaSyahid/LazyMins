# DOKUMENTASI PENYEMPURNAAN SKEMA DATABASE PERCETAKAN

## 📋 RINGKASAN PERUBAHAN

Dokumen ini menjelaskan semua perbaikan dan penambahan yang dilakukan pada skema database aplikasi administrasi percetakan.

---

## 🎯 PERUBAHAN UTAMA

### 1. **PERBAIKAN STRUKTUR TABEL EXISTING**

#### Tabel `konsumen` (Customers)
**TAMBAHAN:**
- `customer_code` - Kode unik pelanggan (auto-generate: CUST-00001)
- `customer_type` - Tipe pelanggan (individual/company)
- `kota` - Kota pelanggan
- `kode_pos` - Kode pos
- `npwp` - NPWP untuk pelanggan korporat
- `catatan` - Catatan khusus pelanggan
- `price_level_id` - Level harga default untuk pelanggan
- `is_active` - Status aktif/non-aktif
- `total_orders` - Total order (auto-update via trigger)
- `total_spent` - Total pembelian (auto-update via trigger)
- `updated_at` - Timestamp update terakhir

**MODIFIKASI:**
- Removed `UNIQUE` constraint pada `email` (tidak semua pelanggan punya email)

**ALASAN:**
- Kode pelanggan memudahkan referensi dan pencarian
- Customer type membedakan pelanggan perorangan vs perusahaan
- NPWP diperlukan untuk pelanggan korporat (faktur pajak)
- Tracking total orders & spending untuk analisis customer loyalty
- Field tambahan meningkatkan profil pelanggan

---

#### Tabel `admins` (Users/Staff)
**TAMBAHAN:**
- `email` - Email admin
- `nomor_telp` - Nomor telepon admin
- `updated_at` - Timestamp update terakhir

**ALASAN:**
- Kontak admin penting untuk komunikasi internal
- Tracking perubahan data admin

---

#### Tabel `products`
**TAMBAHAN:**
- `category_id` - Kategori produk (relasi ke tabel baru)
- `description` - Deskripsi produk
- `unit` - Satuan (pcs, lembar, meter, dll)
- `min_stock` - Minimum stok untuk alert
- `cost_price` - Harga pokok (HPP) untuk kalkulasi profit
- `updated_at` - Timestamp update terakhir

**ALASAN:**
- Kategorisasi memudahkan manajemen produk
- Unit penting untuk produk non-standard
- Min stock untuk sistem alert stok rendah
- Cost price untuk analisis margin keuntungan

---

#### Tabel `price_levels`
**TAMBAHAN:**
- `discount_percentage` - Persentase diskon untuk level
- `description` - Deskripsi level harga
- Level baru: `wholesale` (harga grosir)

**ALASAN:**
- Diskon percentage untuk kalkulasi otomatis
- Level wholesale untuk pelanggan volume besar

---

#### Tabel `finishing_services`
**TAMBAHAN:**
- `code` - Kode unik finishing (FIN-001)
- `description` - Deskripsi layanan
- `unit` - Satuan (pcs, lembar, meter)
- `is_active` - Status aktif/non-aktif
- `created_at` & `updated_at` - Timestamps

**ALASAN:**
- Kode memudahkan referensi
- Unit fleksibel untuk berbagai jenis finishing

---

#### Tabel `orders`
**TAMBAHAN FIELD FINANSIAL:**
- `subtotal` - Subtotal sebelum diskon
- `discount_amount` - Jumlah diskon dalam rupiah
- `discount_percentage` - Persentase diskon
- `tax_amount` - Jumlah pajak (PPN)
- `price_level_id` - Level harga yang digunakan

**TAMBAHAN FIELD STATUS & TRACKING:**
- `priority` - Prioritas order (urgent, high, normal, low)
- `order_date` - Tanggal order dibuat
- `deadline_date` - Deadline pengerjaan
- `completion_date` - Tanggal selesai dikerjakan
- `payment_status` - Status pembayaran (unpaid, partial, paid)
- `paid_amount` - Jumlah yang sudah dibayar

**TAMBAHAN FIELD CATATAN:**
- `notes` - Catatan order (terlihat customer)
- `internal_notes` - Catatan internal (hanya staff)
- `customer_phone` - Nomor telp customer (denormalisasi)

**MODIFIKASI STATUS:**
- Status lebih detail: pending, processing, ready, completed, cancelled

**ALASAN:**
- Tracking finansial lebih detail untuk akuntansi
- Payment status terpisah dari order status
- Priority untuk manajemen antrian produksi
- Deadline tracking untuk SLA
- Notes terpisah untuk customer vs internal

---

#### Tabel `order_items`
**TAMBAHAN:**
- `product_name` - Nama produk (denormalisasi)
- `sku` - SKU produk (denormalisasi)
- `unit` - Satuan
- `discount_percentage` - Diskon per item
- `discount_amount` - Jumlah diskon per item
- `notes` - Catatan khusus item

**ALASAN:**
- Denormalisasi mencegah data hilang jika produk dihapus
- Diskon per item untuk fleksibilitas pricing
- Notes untuk instruksi khusus per item

---

#### Tabel `order_item_finishings`
**TAMBAHAN:**
- `finishing_name` - Nama finishing (denormalisasi)
- `quantity` - Jumlah yang di-finishing
- `subtotal` - Total biaya finishing

**ALASAN:**
- Denormalisasi untuk data historis
- Quantity finishing bisa berbeda dengan quantity item

---

#### Tabel `payments`
**RENAME:**
- `paytime` → `payment_date` (lebih jelas)
- `transfer_acc` → `transfer_account_number` (lebih deskriptif)
- `transfer_ver` → `transfer_verified` (lebih jelas)

**TAMBAHAN:**
- `payment_number` - Nomor pembayaran unik (PAY-00001)
- `customer_id` - Referensi ke customer
- `transfer_bank` - Nama bank
- `transfer_account_name` - Nama pemilik rekening
- `transfer_proof_image` - Path foto bukti transfer
- `cash_received` - Jumlah uang tunai diterima
- `cash_change` - Kembalian
- `payment_status` - Status pembayaran (pending, verified, cancelled)
- `notes` - Catatan pembayaran
- `verified_by` - Admin yang verifikasi
- `verified_at` - Waktu verifikasi
- `updated_at` - Timestamp update

**ALASAN:**
- Payment number untuk tracking dan referensi
- Detail transfer lebih lengkap untuk reconciliation
- Cash handling untuk transaksi tunai
- Verification workflow untuk transfer
- Audit trail lengkap

---

#### Tabel `kategori_transaksi`
**TAMBAHAN:**
- `kode` - Kode kategori (KAT-001)
- `parent_id` - Untuk sub-kategori
- `description` - Deskripsi kategori
- `is_active` - Status aktif/non-aktif

**DATA AWAL:**
- 8 kategori default (4 pemasukan, 4 pengeluaran)

**ALASAN:**
- Hierarki kategori untuk klasifikasi lebih detail
- Kategori default memudahkan setup awal

---

#### Tabel `transaksi`
**TAMBAHAN:**
- `transaction_number` - Nomor transaksi unik
- `tipe` - Tipe transaksi langsung (pemasukan/pengeluaran)
- `payment_method` - Metode pembayaran
- `reference_type` - Tipe referensi (order, payment, expense)
- `reference_id` - ID referensi
- `attachment` - Path file lampiran
- `updated_at` - Timestamp update

**MODIFIKASI:**
- Tipe data `jumlah` dari INTEGER ke REAL (untuk desimal)

**ALASAN:**
- Reference tracking untuk audit trail
- Attachment untuk bukti transaksi
- Tipe langsung memudahkan filtering

---

### 2. **TABEL BARU**

#### `product_categories`
**TUJUAN:** Kategorisasi produk

**FIELD:**
- `id`, `category_name`, `description`, `is_active`, `created_at`

**DATA AWAL:**
- Banner, Brosur, Kartu Nama, Stiker, Fotocopy, Lainnya

**MANFAAT:**
- Organisasi produk lebih baik
- Filtering dan reporting by category
- Analisis penjualan per kategori

---

#### `payment_methods`
**TUJUAN:** Master data metode pembayaran

**FIELD:**
- `id`, `method_code`, `method_name`, `is_active`, `created_at`

**DATA AWAL:**
- Cash, Transfer, QRIS, Debit, Credit

**MANFAAT:**
- Standardisasi metode pembayaran
- Mudah menambah metode baru
- Laporan per metode pembayaran

---

#### `stock_movements`
**TUJUAN:** Tracking pergerakan stok produk

**FIELD:**
- Movement type (in/out/adjustment)
- Quantity, stock before/after
- Reference (order, purchase, adjustment)
- Notes, admin, date

**MANFAAT:**
- Audit trail pergerakan stok
- Identifikasi penyimpangan stok
- Analisis pola penggunaan
- Troubleshooting stok tidak sesuai

---

#### `activity_logs`
**TUJUAN:** Log aktivitas user untuk audit

**FIELD:**
- Admin, action, table_name, record_id
- Old/new value (JSON)
- IP address, user agent, timestamp

**MANFAAT:**
- Audit trail lengkap
- Security monitoring
- Troubleshooting perubahan data
- Compliance requirements

---

#### `app_settings`
**TUJUAN:** Konfigurasi aplikasi

**FIELD:**
- Setting key/value, data type
- Description, is_public
- Updated by, updated at

**DATA AWAL:**
- Company info, tax %, currency
- Prefixes untuk numbering
- System settings

**MANFAAT:**
- Konfigurasi terpusat
- Mudah diubah tanpa code change
- Multi-tenant ready

---

### 3. **VIEWS (Laporan Instan)**

#### `v_order_summary`
**TUJUAN:** Ringkasan order dengan kalkulasi
**DATA:** Order info, payment info, admin, jumlah items

#### `v_low_stock_products`
**TUJUAN:** Produk dengan stok rendah
**DATA:** Produk dengan stock <= min_stock

#### `v_daily_sales`
**TUJUAN:** Laporan penjualan harian
**DATA:** Total orders, sales, received, outstanding per hari

#### `v_top_products`
**TUJUAN:** Produk terlaris
**DATA:** Produk sorted by revenue

#### `v_top_customers`
**TUJUAN:** Customer loyalty analysis
**DATA:** Customer dengan tier (VIP, Gold, Silver, Regular)

**MANFAAT:**
- Query kompleks jadi sederhana
- Performance lebih baik untuk reporting
- Konsistensi kalkulasi
- Mudah digunakan di aplikasi

---

### 4. **TRIGGERS (Otomasi)**

#### Timestamp Auto-Update
**TRIGGER:** `trg_*_updated_at`
**FUNGSI:** Auto-update field `updated_at` saat data berubah

#### Auto-Generate Customer Code
**TRIGGER:** `trg_konsumen_generate_code`
**FUNGSI:** Generate kode pelanggan otomatis (CUST-00001)

#### Stock Management
**TRIGGER:** `trg_order_items_reduce_stock`
**FUNGSI:** 
- Kurangi stok saat order item dibuat
- Catat ke stock_movements

**TRIGGER:** `trg_orders_cancel_restore_stock`
**FUNGSI:** Kembalikan stok saat order dibatalkan

#### Payment Status Update
**TRIGGER:** `trg_payments_update_order_status`
**FUNGSI:**
- Update paid_amount di order
- Update payment_status (unpaid/partial/paid)

#### Customer Statistics
**TRIGGER:** `trg_orders_update_customer_stats`
**FUNGSI:** Update total_orders & total_spent customer

**MANFAAT:**
- Data selalu konsisten
- Mengurangi logic di aplikasi
- Prevent human error
- Atomicity terjaga

---

### 5. **INDEXES (Optimasi Query)**

**INDEXES BARU:**
- Customer: nama, telp, code
- Admin: username, role
- Product: name, category, sku
- Order: number, customer, status, date, deadline, payment_status
- Payment: order, customer, date, status, method
- Transaksi: kategori, tipe, tanggal
- Stock movements: product, date, type

**MANFAAT:**
- Query lebih cepat 10-100x
- Pencarian real-time smooth
- Laporan generate lebih cepat
- Scalable untuk data besar

---

## 🔄 MIGRASI DARI SKEMA LAMA

### Langkah Migrasi:

```sql
-- 1. Backup database lama
-- 2. Buat database baru dengan skema improved
-- 3. Migrasi data:

-- Konsumen
INSERT INTO konsumen (id, nama_lengkap, email, nomor_telp, alamat, last_seen, created_at)
SELECT id, nama_lengkap, email, nomor_telp, alamat, last_seen, created_at
FROM old_database.konsumen;

-- Products
INSERT INTO products (id, sku, name, stock, is_active)
SELECT id, sku, name, stock, is_active
FROM old_database.products;

-- dst... untuk semua tabel
```

---

## 📊 FITUR BARU YANG BISA DIIMPLEMENTASI

### 1. **Customer Loyalty Program**
- Tier otomatis berdasarkan total_spent
- Point rewards
- Special pricing per tier

### 2. **Inventory Management**
- Low stock alerts
- Auto-reorder
- Stock opname

### 3. **Financial Reports**
- Profit/Loss statement
- Cash flow
- AR/AP aging
- Tax reports

### 4. **Production Management**
- Queue management by priority
- Deadline alerts
- Workload balancing

### 5. **Analytics Dashboard**
- Sales trends
- Product performance
- Customer behavior
- Revenue forecasting

### 6. **Notifications**
- Low stock alerts
- Payment reminders
- Deadline warnings
- Daily summaries

---

## 🎨 BEST PRACTICES YANG DITERAPKAN

### 1. **Normalisasi Data**
✅ Tabel terpisah untuk master data
✅ Foreign keys untuk integritas
✅ Minimal redundansi

### 2. **Denormalisasi Strategis**
✅ Customer name di orders (untuk performa)
✅ Product info di order_items (untuk historis)
✅ Finishing name di order_item_finishings

### 3. **Audit Trail**
✅ created_at & updated_at di semua tabel
✅ Activity logs untuk perubahan penting
✅ Admin tracking di setiap transaksi

### 4. **Data Integrity**
✅ Foreign keys dengan ON DELETE CASCADE/RESTRICT
✅ CHECK constraints untuk validasi
✅ NOT NULL untuk field mandatory
✅ UNIQUE constraints untuk kode-kode

### 5. **Performance**
✅ Indexes pada field yang sering di-query
✅ Views untuk query kompleks
✅ Triggers untuk kalkulasi otomatis

### 6. **Flexibility**
✅ Settings table untuk konfigurasi
✅ Status/type sebagai TEXT (mudah extend)
✅ Notes field untuk informasi tambahan

---

## 🚀 CARA PENGGUNAAN

### Setup Database Baru:
```bash
sqlite3 percetakan.db < percetakan_schema_improved.sql
```

### Testing:
```sql
-- Cek struktur
.schema

-- Cek data awal
SELECT * FROM roles;
SELECT * FROM price_levels;
SELECT * FROM product_categories;

-- Test views
SELECT * FROM v_order_summary;
SELECT * FROM v_low_stock_products;
```

### Aplikasi Development:
- Gunakan ORM (SQLAlchemy, Django ORM, dll)
- Atau query builder untuk safety
- Implement proper error handling
- Validate input sebelum insert

---

## 📝 CATATAN PENTING

1. **Backup Rutin**
   - Daily automated backup
   - Simpan di lokasi terpisah
   - Test restore procedure

2. **Security**
   - Password harus di-hash (bcrypt/argon2)
   - Sanitize input untuk prevent SQL injection
   - Implement rate limiting
   - Audit logs untuk monitoring

3. **Maintenance**
   - VACUUM database secara berkala
   - ANALYZE untuk update statistics
   - Monitor ukuran database
   - Archive data lama

4. **Future Enhancements**
   - Multi-currency support
   - Multi-warehouse
   - Integration API
   - Mobile app support
   - WhatsApp notifications

---

## ❓ FAQ

**Q: Apakah bisa langsung replace database lama?**
A: Tidak disarankan. Lakukan migrasi bertahap dan testing.

**Q: Apakah perlu semua field tambahan?**
A: Sesuaikan dengan kebutuhan. Field tambahan bisa di-drop jika tidak diperlukan.

**Q: Bagaimana dengan performa untuk data besar?**
A: Skema sudah dioptimasi dengan indexes. Untuk data >1juta rows, pertimbangkan partitioning.

**Q: Bisa dipakai untuk multi-cabang?**
A: Ya, tambahkan tabel `branches` dan `branch_id` di tabel relevan.

---

## 📞 SUPPORT

Jika ada pertanyaan atau issue:
1. Cek dokumentasi ini
2. Review skema SQL
3. Test di development environment dulu
4. Backup sebelum implementasi ke production

---

**Version:** 2.0
**Last Updated:** 2026-02-05
**Author:** Database Improvement Team