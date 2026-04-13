import sqlite3
import os
import traceback

# Konfigurasi file skema
SCHEMA_FILE = r'H:\QtProject\LazyMins\resources\schema\percetakan_schema_improved.sql'

def setup_database():
    """Membuat database di memory dan memuat skema."""
    if not os.path.exists(SCHEMA_FILE):
        raise FileNotFoundError(f"File {SCHEMA_FILE} tidak ditemukan. Pastikan nama file sesuai.")
    
    conn = sqlite3.connect('mem.db')
    conn.row_factory = sqlite3.Row  # Agar hasil query bisa diakses via nama kolom
    conn.execute("PRAGMA foreign_keys = ON;")
    
    with open(SCHEMA_FILE, 'r', encoding='utf-8') as f:
        schema = f.read()
    
    conn.executescript(schema)
    return conn

def print_header(title):
    print(f"\n{'-'*60}")
    print(f"🔄 MENGUJI: {title}")
    print(f"{'-'*60}")

def run_tests():
    conn = setup_database()
    cursor = conn.cursor()
    
    try:
        # Suntikkan admin dummy karena di schema.sql sengaja di-comment
        cursor.execute("""
            INSERT INTO admins (id, role_id, username, password_hash, salt, nama_lengkap) 
            VALUES (1, 1, 'test_admin', 'hash', 'salt', 'Admin Penguji')
        """)
        # =====================================================================
        print_header("1. Cek Data Awal (Seeding)")
        # =====================================================================
        cursor.execute("SELECT stock FROM products WHERE sku = 'BN-FLEX'")
        initial_stock = cursor.fetchone()['stock']
        print(f"✅ Stok awal BN-FLEX: {initial_stock}")
        assert initial_stock == 120, "Stok awal BN-FLEX harusnya 120"
        
        # =====================================================================
        print_header("2. Test Pembuatan Order & Pemotongan Stok (Trigger Insert)")
        # =====================================================================
        # Buat Order Baru
        cursor.execute("""
            INSERT INTO orders (order_number, customer_id, customer_name, admin_id) 
            VALUES ('ORD-001', 1, 'PT. Sinar Jaya', 1)
        """)
        order_id = cursor.lastrowid
        
        # Masukkan Item (10 pcs x 2m x 1m = 20 meter area)
        sale_price = 25000
        qty = 10
        width = 2
        height = 1
        
        cursor.execute("""
            INSERT INTO order_items (order_id, product_id, product_name, sku, quantity, unit, size_width, size_height, use_area, sale_price, base_price)
            VALUES (?, 1, 'FLEXY', 'BN-FLEX', ?, 'meter', ?, ?, 1, ?, 15000)
        """, (order_id, qty, width, height, sale_price))
        item_id = cursor.lastrowid
        
        # Cek pemotongan stok
        cursor.execute("SELECT stock FROM products WHERE sku = 'BN-FLEX'")
        new_stock = cursor.fetchone()['stock']
        expected_deduction = qty * width * height
        print(f"✅ Stok setelah order: {new_stock} (Berkurang {expected_deduction})")
        assert new_stock == (initial_stock - expected_deduction), "Pemotongan stok tidak sesuai dimensi!"
        
        # Cek log stock_movements
        cursor.execute("SELECT * FROM stock_movements WHERE reference_id = ? AND reference_type = 'order'", (order_id,))
        movement = cursor.fetchone()
        assert movement['movement_type'] == 'out', "Tipe movement harus 'out'"
        assert movement['quantity'] == expected_deduction, "Quantity di log pergerakan stok salah"
        print("✅ Log stock_movements tercatat dengan benar.")

        # =====================================================================
        print_header("3. Test Update Item Order & Sync Stok (Trigger Update Item)")
        # =====================================================================
        # Ubah qty dari 10 menjadi 15 (Tambah 5 pcs x 2m x 1m = 10 meter area)
        new_qty = 15
        cursor.execute("UPDATE order_items SET quantity = ? WHERE id = ?", (new_qty, item_id))
        
        cursor.execute("SELECT stock FROM products WHERE sku = 'BN-FLEX'")
        updated_stock = cursor.fetchone()['stock']
        expected_stock_after_update = initial_stock - (new_qty * width * height)
        
        print(f"✅ Stok setelah edit qty: {updated_stock}")
        assert updated_stock == expected_stock_after_update, "Trigger update item gagal sinkronisasi stok utama!"
        
        # =====================================================================
        print_header("4. Test Kalkulasi Finishing (Trigger Finishing)")
        # =====================================================================
        finishing_qty = 15
        finishing_price = 4000
        
        # Tambah Finishing
        cursor.execute("""
            INSERT INTO order_item_finishings (order_item_id, finishing_id, finishing_name, quantity, finishing_price)
            VALUES (?, 1, 'L DOFF', ?, ?)
        """, (item_id, finishing_qty, finishing_price))
        
        # Cek finishing_total di order_items
        cursor.execute("SELECT finishing_total, total FROM order_items WHERE id = ?", (item_id,))
        item_data = cursor.fetchone()
        expected_finishing_total = finishing_qty * finishing_price
        assert item_data['finishing_total'] == expected_finishing_total, "finishing_total tidak terupdate di order_items"
        
        # Cek total order
        cursor.execute("SELECT subtotal FROM orders WHERE id = ?", (order_id,))
        order_data = cursor.fetchone()
        expected_subtotal = item_data['total']
        assert order_data['subtotal'] == expected_subtotal, "subtotal order tidak sinkron dengan total item"
        print(f"✅ Kalkulasi Subtotal Item & Order berhasil di-cascade. Total: Rp {order_data['subtotal']}")

        # =====================================================================
        print_header("5. Test Pembuatan Invoice & Penyatuan Order (Trigger Invoice)")
        # =====================================================================
        # Buat Invoice Kosong
        cursor.execute("""
            INSERT INTO invoices (invoice_number, customer_id, customer_name, admin_id)
            VALUES ('INV-TEST-001', 1, 'PT. Sinar Jaya', 1)
        """)
        invoice_id = cursor.lastrowid
        
        # Tautkan Order ke Invoice
        cursor.execute("UPDATE orders SET invoice_id = ? WHERE id = ?", (invoice_id, order_id))
        
        # Cek Total Invoice
        cursor.execute("SELECT subtotal, total_amount FROM invoices WHERE id = ?", (invoice_id,))
        invoice_data = cursor.fetchone()
        assert invoice_data['subtotal'] == expected_subtotal, "Subtotal invoice tidak sinkron dengan order"
        print(f"✅ Invoice terbuat dan otomatis mengambil nilai order. Total: Rp {invoice_data['total_amount']}")

        # =====================================================================
        print_header("6. Test Pembayaran & Sync Status Lunas (Trigger Payment)")
        # =====================================================================
        payment_amount = invoice_data['total_amount']
        
        # Masukkan Pembayaran Lunas
        cursor.execute("""
            INSERT INTO payments (payment_number, invoice_id, amount, admin_id)
            VALUES ('PAY-001', ?, ?, 1)
        """, (invoice_id, payment_amount))
        
        # Cek Invoice Paid Amount & Remaining
        cursor.execute("SELECT paid_amount, remaining_amount FROM invoices WHERE id = ?", (invoice_id,))
        inv_after_pay = cursor.fetchone()
        assert inv_after_pay['paid_amount'] == payment_amount, "paid_amount di invoice salah"
        assert inv_after_pay['remaining_amount'] == 0, "Sisa tagihan (remaining_amount) harus 0"
        
        # Cek Sync Status Order (Trigger baru)
        cursor.execute("SELECT payment_status FROM orders WHERE id = ?", (order_id,))
        order_status = cursor.fetchone()['payment_status']
        assert order_status == 'paid', "Status order gagal berubah menjadi 'paid' secara otomatis"
        print(f"✅ Pembayaran Rp {payment_amount} sukses. Status order otomatis menjadi LUNAS ('paid').")

        # =====================================================================
        print_header("7. Test Penghapusan Order Item & Restore Stok (Trigger Delete)")
        # =====================================================================
        # Hapus item dari order
        cursor.execute("DELETE FROM order_items WHERE id = ?", (item_id,))
        
        # Cek pengembalian stok
        cursor.execute("SELECT stock FROM products WHERE sku = 'BN-FLEX'")
        restored_stock = cursor.fetchone()['stock']
        assert restored_stock == initial_stock, "Stok tidak kembali ke angka awal setelah item dihapus"
        
        # Cek subtotal order menjadi 0
        cursor.execute("SELECT subtotal FROM orders WHERE id = ?", (order_id,))
        empty_order_subtotal = cursor.fetchone()['subtotal']
        assert empty_order_subtotal == 0, "Subtotal order harusnya 0 setelah item dihapus"
        print("✅ Penghapusan item berhasil mengembalikan stok ke awal dan me-reset subtotal order.")

        print("\n🎉 SEMUA PENGUJIAN DATABASE BERHASIL DIJALANKAN TANPA ERROR! 🎉")

    except AssertionError as e:
        print(f"\n❌ PENGUJIAN GAGAL (AssertionError): {e}")
    except Exception as e:
        print(f"\n❌ TERJADI ERROR PADA SQLITE:")
        traceback.print_exc()
    finally:
        conn.close()

if __name__ == '__main__':
    run_tests()