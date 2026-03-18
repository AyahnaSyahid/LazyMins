"""
================================================================================
GENERATOR NOTA PENJUALAN - TEXT BASED
================================================================================
Contoh implementasi Python untuk generate nota berbasis text
yang bisa dicetak ke printer thermal, dot matrix, atau export ke file
================================================================================
"""

from datetime import datetime
from typing import List, Dict, Optional
import textwrap


class NotaGenerator:
    """
    Class untuk generate nota penjualan dalam format text
    """
    
    def __init__(self, width: int = 80):
        """
        Initialize generator
        
        Args:
            width: Lebar karakter per baris (40, 80, atau custom)
        """
        self.width = width
        
    def format_currency(self, amount: float) -> str:
        """Format angka ke format Rupiah"""
        return f"Rp {amount:,.2f}".replace(',', '.')
    
    def format_date(self, date: datetime) -> str:
        """Format tanggal ke bahasa Indonesia"""
        months = [
            'Januari', 'Februari', 'Maret', 'April', 'Mei', 'Juni',
            'Juli', 'Agustus', 'September', 'Oktober', 'November', 'Desember'
        ]
        return f"{date.day:02d} {months[date.month-1]} {date.year}"
    
    def format_time(self, time: datetime) -> str:
        """Format waktu"""
        return time.strftime("%H:%M WIB")
    
    def center(self, text: str) -> str:
        """Center align text"""
        return text.center(self.width)
    
    def left_align(self, left_text: str, right_text: str) -> str:
        """Align text kiri dan kanan"""
        spaces = self.width - len(left_text) - len(right_text)
        return left_text + (' ' * spaces) + right_text
    
    def separator(self, char: str = '=') -> str:
        """Buat garis separator"""
        return char * self.width
    
    def wrap_text(self, text: str, indent: int = 0) -> str:
        """Wrap text panjang"""
        wrapper = textwrap.TextWrapper(
            width=self.width - indent,
            initial_indent=' ' * indent,
            subsequent_indent=' ' * indent
        )
        return '\n'.join(wrapper.wrap(text))


    def generate_simple_nota(self, data: Dict) -> str:
        """
        Generate nota format simple (40-80 karakter)
        
        Args:
            data: Dictionary berisi data nota
            
        Returns:
            String nota siap print
        """
        nota = []
        
        # Header
        nota.append(self.separator('='))
        nota.append(self.center(data['company']['name']))
        nota.append(self.separator('='))
        nota.append(self.center(data['company']['address']))
        nota.append(self.center(f"Telp: {data['company']['phone']}"))
        if data['company'].get('whatsapp'):
            nota.append(self.center(f"WA: {data['company']['whatsapp']}"))
        nota.append('')
        
        nota.append(self.separator('='))
        nota.append(self.left_align('NO NOTA', f": {data['order_number']}"))
        nota.append(self.left_align('TANGGAL', 
            f": {self.format_date(data['order_date'])}, {self.format_time(data['order_date'])}"))
        nota.append(self.left_align('KASIR', f": {data['admin_name']}"))
        nota.append(self.separator('='))
        nota.append('')
        
        # Customer
        nota.append('PELANGGAN')
        nota.append(self.left_align('Nama', f": {data['customer']['name']}"))
        if data['customer'].get('phone'):
            nota.append(self.left_align('Telp', f": {data['customer']['phone']}"))
        nota.append('')
        
        # Items
        nota.append(self.separator('-'))
        nota.append('ITEM PESANAN')
        nota.append(self.separator('-'))
        
        for item in data['items']:
            # Nama item
            nota.append(item['name'])
            
            # Quantity dan harga
            qty_text = f"  {item['quantity']} {item['unit']} x {self.format_currency(item['price'])}"
            subtotal_text = self.format_currency(item['subtotal'])
            nota.append(self.left_align(qty_text, subtotal_text))
            
            # Finishing jika ada
            if item.get('finishings'):
                nota.append('')
                for finishing in item['finishings']:
                    finishing_text = f"  - {finishing['name']}"
                    price_text = self.format_currency(finishing['price'])
                    nota.append(self.left_align(finishing_text, price_text))
            
            nota.append('')
        
        # Total
        nota.append(self.separator('-'))
        nota.append(self.left_align('', f"SUBTOTAL {self.format_currency(data['subtotal'])}").rjust(self.width))
        
        if data.get('discount', 0) > 0:
            nota.append(self.left_align('', f"DISKON {self.format_currency(data['discount'])}").rjust(self.width))
        
        if data.get('tax', 0) > 0:
            tax_label = f"PPN {data.get('tax_percentage', 11)}%"
            nota.append(self.left_align('', f"{tax_label} {self.format_currency(data['tax'])}").rjust(self.width))
        
        nota.append(self.separator('-'))
        nota.append(self.left_align('', f"TOTAL {self.format_currency(data['total'])}").rjust(self.width))
        nota.append(self.separator('='))
        nota.append('')
        
        # Payment
        nota.append('PEMBAYARAN')
        nota.append(self.left_align(data['payment']['method'], self.format_currency(data['payment']['amount'])))
        
        if data['payment'].get('change'):
            nota.append(self.left_align('Kembali', self.format_currency(data['payment']['change'])))
        
        nota.append('')
        nota.append(self.separator('='))
        nota.append(self.center('Terima kasih atas kepercayaan'))
        nota.append(self.center('Anda kepada kami!'))
        nota.append('')
        nota.append(self.center('Barang yang sudah dibeli'))
        nota.append(self.center('tidak dapat dikembalikan'))
        nota.append(self.separator('='))
        
        return '\n'.join(nota)


    def generate_detailed_nota(self, data: Dict) -> str:
        """
        Generate nota format detailed dengan breakdown lengkap
        """
        nota = []
        
        # Header dengan box
        nota.append(self.separator('='))
        nota.append(self.center(data['company']['name'].upper()))
        nota.append(self.separator('='))
        nota.append(self.center(data['company']['address']))
        nota.append(self.center(f"Telp: {data['company']['phone']} | WA: {data['company']['whatsapp']}"))
        nota.append(self.center(f"Email: {data['company']['email']}"))
        if data['company'].get('npwp'):
            nota.append(self.center(f"NPWP: {data['company']['npwp']}"))
        nota.append(self.separator('='))
        nota.append('')
        
        # Info Nota
        nota.append('NOTA PENJUALAN')
        nota.append('')
        nota.append(self.left_align(f"No. Nota    : {data['order_number']}", 
                                   f"Tanggal : {self.format_date(data['order_date'])}"))
        nota.append(self.left_align(f"Waktu       : {self.format_time(data['order_date'])}", 
                                   f"Kasir   : {data['admin_name']}"))
        nota.append(self.left_align(f"Status      : {data.get('status', 'LUNAS')}", ''))
        nota.append('')
        
        # Customer Info
        nota.append(self.separator('-'))
        nota.append('PELANGGAN')
        nota.append(self.separator('-'))
        nota.append(self.left_align('Nama', f": {data['customer']['name']}"))
        if data['customer'].get('phone'):
            nota.append(self.left_align('No. Telp', f": {data['customer']['phone']}"))
        if data['customer'].get('address'):
            nota.append(self.left_align('Alamat', f": {data['customer']['address']}"))
        if data['customer'].get('email'):
            nota.append(self.left_align('Email', f": {data['customer']['email']}"))
        nota.append('')
        
        # Items Detail
        nota.append(self.separator('='))
        nota.append('DETAIL PESANAN')
        nota.append(self.separator('='))
        nota.append(f"{'No':<4} {'Nama Item':<30} {'Qty':>5} {'Satuan':<7} {'Harga Satuan':>15} {'Subtotal':>15}")
        nota.append(self.separator('-'))
        
        item_no = 1
        for item in data['items']:
            # Item header
            nota.append(f"{item_no:<4} {item['name'][:30]:<30} {item['quantity']:>5} {item['unit']:<7} "
                       f"{self.format_currency(item['price']):>15} {self.format_currency(item['subtotal']):>15}")
            
            # Item description
            if item.get('description'):
                nota.append(f"     {item['description']}")
            
            # Finishings
            if item.get('finishings'):
                nota.append('')
                nota.append('     Finishing:')
                for finishing in item['finishings']:
                    f_qty = finishing.get('quantity', item['quantity'])
                    f_text = f"     - {finishing['name']:<40} {f_qty:>5} {item['unit']:<7}"
                    f_price = f"{self.format_currency(finishing['unit_price']):>15} {self.format_currency(finishing['subtotal']):>15}"
                    nota.append(f_text + f_price)
            
            nota.append('')
            item_no += 1
        
        # Summary
        nota.append(self.separator('-'))
        nota.append(self.left_align('', f"SUBTOTAL    {self.format_currency(data['subtotal'])}"))
        
        if data.get('discount', 0) > 0:
            discount_pct = f"({data.get('discount_percentage', 0)}%)" if data.get('discount_percentage') else ''
            nota.append(self.left_align('', f"DISKON {discount_pct}     ({self.format_currency(data['discount'])})"))
        
        if data.get('discount', 0) > 0:
            subtotal_after = data['subtotal'] - data['discount']
            nota.append(self.left_align('', (' ' * 11) + self.separator('-')[:25]))
            nota.append(self.left_align('', f"SUBTOTAL AFTER DISC   {self.format_currency(subtotal_after)}"))
        
        if data.get('tax', 0) > 0:
            tax_label = f"PPN {data.get('tax_percentage', 11)}%"
            nota.append(self.left_align('', f"{tax_label}      {self.format_currency(data['tax'])}"))
        
        nota.append(self.separator('-'))
        nota.append(self.left_align('', f"TOTAL TAGIHAN     {self.format_currency(data['total'])}"))
        nota.append(self.separator('='))
        nota.append('')
        
        # Payment Details
        nota.append('RINCIAN PEMBAYARAN')
        nota.append(self.separator('-'))
        nota.append(self.left_align('Metode', f": {data['payment']['method']}"))
        nota.append(self.left_align('Jumlah Bayar', f": {self.format_currency(data['payment']['amount'])}"))
        
        if data['payment'].get('change'):
            nota.append(self.left_align('Kembalian', f": {self.format_currency(data['payment']['change'])}"))
        
        nota.append('')
        nota.append(f"STATUS PEMBAYARAN: {data.get('payment_status', 'LUNAS')} ✓")
        nota.append('')
        
        # Estimasi
        if data.get('deadline_date'):
            nota.append(self.separator('='))
            nota.append(f"ESTIMASI SELESAI: {self.format_date(data['deadline_date'])}")
            nota.append(self.separator('-'))
        
        # Notes
        nota.append('')
        nota.append('CATATAN:')
        nota.append('- Barang yang sudah dipesan tidak dapat dibatalkan')
        nota.append('- Harap membawa nota ini saat pengambilan barang')
        nota.append('- Komplain paling lambat 2x24 jam setelah barang diambil')
        nota.append('')
        
        nota.append(self.separator('='))
        nota.append(self.center('Terima kasih atas kepercayaan Anda kepada kami!'))
        nota.append(self.center('Kepuasan Anda adalah prioritas kami'))
        nota.append(self.separator('='))
        
        # Signature
        nota.append('')
        nota.append('')
        left_sig = 'Tanda Tangan Kasir'
        right_sig = 'Tanda Tangan Pelanggan'
        nota.append(self.left_align(f"    {left_sig}", f"{right_sig}    "))
        nota.append('')
        nota.append('')
        nota.append(self.left_align(f"    {'_' * 21}", f"{'_' * 21}    "))
        nota.append(self.left_align(f"       {data['admin_name']}", f"{data['customer']['name']}       "))
        
        return '\n'.join(nota)


def example_usage():
    """
    Contoh penggunaan NotaGenerator
    """
    
    # Sample data
    order_data = {
        'company': {
            'name': 'PERCETAKAN MAJU JAYA',
            'address': 'Jl. Raya Indah No. 123, Tasikmalaya',
            'phone': '(0265) 123-456',
            'whatsapp': '0812-3456-7890',
            'email': 'info@percetakanmajujaya.com',
            'npwp': '01.234.567.8-901.000'
        },
        'order_number': 'ORD-20260205-0001',
        'order_date': datetime.now(),
        'deadline_date': datetime(2026, 2, 7, 16, 0),
        'admin_name': 'Budi Santoso',
        'status': 'PROCESSING',
        'payment_status': 'LUNAS',
        'customer': {
            'name': 'Ibu Siti Rahayu',
            'phone': '0812-9999-8888',
            'address': 'Jl. Merdeka No. 45, Tasikmalaya',
            'email': 'siti.rahayu@email.com'
        },
        'items': [
            {
                'name': 'Banner Flexi 340 gram',
                'description': 'Ukuran: 2x1 meter',
                'quantity': 2,
                'unit': 'pcs',
                'price': 75000,
                'subtotal': 150000,
                'finishings': []
            },
            {
                'name': 'Kartu Nama Art Paper 260gr',
                'description': 'Ukuran: 9x5.5 cm, Cetak: Full Color 2 Sisi',
                'quantity': 500,
                'unit': 'pcs',
                'price': 100,
                'subtotal': 50000,
                'finishings': [
                    {
                        'name': 'Laminating Doff',
                        'quantity': 500,
                        'unit_price': 20,
                        'subtotal': 10000
                    },
                    {
                        'name': 'Cutting Precision',
                        'quantity': 500,
                        'unit_price': 10,
                        'subtotal': 5000
                    }
                ]
            }
        ],
        'subtotal': 215000,
        'discount': 10000,
        'discount_percentage': 0,
        'tax': 22550,
        'tax_percentage': 11,
        'total': 227550,
        'payment': {
            'method': 'Tunai (Cash)',
            'amount': 230000,
            'change': 2450
        }
    }
    
    # Generate nota simple (width 80)
    generator = NotaGenerator(width=80)
    
    print("=" * 80)
    print("NOTA FORMAT SIMPLE")
    print("=" * 80)
    nota_simple = generator.generate_simple_nota(order_data)
    print(nota_simple)
    print("\n\n")
    
    print("=" * 80)
    print("NOTA FORMAT DETAILED")
    print("=" * 80)
    nota_detailed = generator.generate_detailed_nota(order_data)
    print(nota_detailed)
    
    # Save to file
    with open('nota_output_simple.txt', 'w', encoding='utf-8') as f:
        f.write(nota_simple)
    
    with open('nota_output_detailed.txt', 'w', encoding='utf-8') as f:
        f.write(nota_detailed)
    
    print("\n\n✓ Nota berhasil di-generate dan disimpan ke file!")


# ================================================================================
# INTEGRASI DENGAN DATABASE
# ================================================================================

def generate_nota_from_database(order_id: int, db_path: str = 'percetakan.db') -> str:
    """
    Generate nota langsung dari database
    
    Args:
        order_id: ID order dari database
        db_path: Path ke database SQLite
        
    Returns:
        String nota siap print
    """
    import sqlite3
    
    # Connect ke database
    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    
    # Query order data
    cursor.execute("""
        SELECT 
            o.*,
            k.nama_lengkap as customer_name,
            k.nomor_telp as customer_phone,
            k.alamat as customer_address,
            k.email as customer_email,
            a.nama_lengkap as admin_name
        FROM orders o
        JOIN konsumen k ON o.customer_id = k.id
        JOIN admins a ON o.admin_id = a.id
        WHERE o.id = ?
    """, (order_id,))
    
    order = cursor.fetchone()
    if not order:
        return "Order tidak ditemukan!"
    
    # Query order items
    cursor.execute("""
        SELECT 
            oi.*,
            p.name as product_name,
            p.unit
        FROM order_items oi
        LEFT JOIN products p ON oi.product_id = p.id
        WHERE oi.order_id = ?
    """, (order_id,))
    
    items = []
    for item_row in cursor.fetchall():
        # Query finishings untuk item ini
        cursor.execute("""
            SELECT 
                oif.*,
                fs.name as finishing_name
            FROM order_item_finishings oif
            LEFT JOIN finishing_services fs ON oif.finishing_id = fs.id
            WHERE oif.order_item_id = ?
        """, (item_row['id'],))
        
        finishings = []
        for f_row in cursor.fetchall():
            finishings.append({
                'name': f_row['finishing_name'] or f_row['finishing_name'],
                'quantity': f_row.get('quantity', item_row['quantity']),
                'unit_price': f_row['finishing_price'],
                'subtotal': f_row.get('subtotal', f_row['finishing_price'])
            })
        
        items.append({
            'name': item_row['product_name'] or item_row['product_name'],
            'description': item_row.get('notes', ''),
            'quantity': item_row['quantity'],
            'unit': item_row.get('unit', 'pcs'),
            'price': item_row['base_price'],
            'subtotal': item_row['subtotal'],
            'finishings': finishings
        })
    
    # Query payment info
    cursor.execute("""
        SELECT 
            payment_method,
            SUM(amount) as total_paid
        FROM payments
        WHERE order_id = ? AND payment_status = 'verified'
        GROUP BY payment_method
        LIMIT 1
    """, (order_id,))
    
    payment_row = cursor.fetchone()
    
    # Query company settings
    cursor.execute("""
        SELECT setting_key, setting_value
        FROM app_settings
        WHERE setting_key IN ('company_name', 'company_address', 'company_phone', 
                             'company_email', 'company_npwp', 'company_whatsapp')
    """)
    
    settings = {row['setting_key']: row['setting_value'] for row in cursor.fetchall()}
    
    conn.close()
    
    # Build data structure
    order_data = {
        'company': {
            'name': settings.get('company_name', 'PERCETAKAN'),
            'address': settings.get('company_address', ''),
            'phone': settings.get('company_phone', ''),
            'whatsapp': settings.get('company_whatsapp', ''),
            'email': settings.get('company_email', ''),
            'npwp': settings.get('company_npwp', '')
        },
        'order_number': order['order_number'],
        'order_date': datetime.fromisoformat(order['order_date']),
        'deadline_date': datetime.fromisoformat(order['deadline_date']) if order.get('deadline_date') else None,
        'admin_name': order['admin_name'],
        'status': order['status'].upper(),
        'payment_status': order['payment_status'].upper(),
        'customer': {
            'name': order['customer_name'],
            'phone': order.get('customer_phone', ''),
            'address': order.get('customer_address', ''),
            'email': order.get('customer_email', '')
        },
        'items': items,
        'subtotal': order.get('subtotal', 0) or order['total_amount'],
        'discount': order.get('discount_amount', 0),
        'discount_percentage': order.get('discount_percentage', 0),
        'tax': order.get('tax_amount', 0),
        'tax_percentage': 11,
        'total': order['total_amount'],
        'payment': {
            'method': payment_row['payment_method'] if payment_row else 'Cash',
            'amount': payment_row['total_paid'] if payment_row else order['total_amount'],
            'change': 0
        }
    }
    
    # Calculate change if cash
    if payment_row and payment_row['payment_method'] == 'cash':
        order_data['payment']['change'] = max(0, payment_row['total_paid'] - order['total_amount'])
    
    # Generate nota
    generator = NotaGenerator(width=80)
    return generator.generate_detailed_nota(order_data)


# ================================================================================
# PRINT TO THERMAL PRINTER (ESC/POS)
# ================================================================================

def print_to_thermal(nota_text: str, printer_name: str = None):
    """
    Print nota ke thermal printer menggunakan ESC/POS
    Requires: python-escpos
    
    Install: pip install python-escpos
    """
    try:
        from escpos.printer import Usb, Network, File
        
        # Example untuk USB printer
        # p = Usb(0x04b8, 0x0e15)  # Sesuaikan vendor_id dan product_id
        
        # Example untuk Network printer
        # p = Network("192.168.1.100")
        
        # Example untuk save to file
        p = File("/dev/usb/lp0")  # Atau path printer Anda
        
        # Set mode
        p.set(align='center', text_type='B')
        p.text("PERCETAKAN MAJU JAYA\n")
        p.set(align='left', text_type='normal')
        
        # Print nota
        p.text(nota_text)
        
        # Cut paper
        p.cut()
        
        print("✓ Nota berhasil dicetak!")
        
    except ImportError:
        print("Error: Install python-escpos terlebih dahulu")
        print("pip install python-escpos")
    except Exception as e:
        print(f"Error printing: {e}")


if __name__ == "__main__":
    # Run example
    example_usage()
    
    # Uncomment untuk test dari database
    # nota = generate_nota_from_database(order_id=1)
    # print(nota)
