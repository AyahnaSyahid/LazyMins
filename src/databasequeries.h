#ifndef DATABASEQUERIES_H
#define DATABASEQUERIES_H

#include <QString>

class DatabaseQueries {
public:
    // Tabel users
    static const QString createUsersTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS users (
                user_id INTEGER PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                salt TEXT NOT NULL, 
                password_hash TEXT NOT NULL,
                full_name TEXT,
                email TEXT,
                phone_number TEXT,
                home_address TEXT,
                is_active BOOLEAN DEFAULT 1,
                last_active TIMESTAMP,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )";
    }

    // Tabel roles
    static const QString createRolesTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS roles (
                role_id INTEGER PRIMARY KEY,
                role_name TEXT UNIQUE NOT NULL,
                description TEXT
            );
        )";
    }

    // Tabel user_roles
    static const QString createUserRolesTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS user_roles (
                user_id INTEGER,
                role_id INTEGER,
                assigned_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                PRIMARY KEY (user_id, role_id),
                FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
                FOREIGN KEY (role_id) REFERENCES roles(role_id) ON DELETE CASCADE
            );
        )";
    }

    // Tabel permissions
    static const QString createPermissionsTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS permissions (
                permission_id INTEGER PRIMARY KEY,
                permission_name TEXT UNIQUE NOT NULL,
                description TEXT
            );
        )";
    }

    // Tabel role_permissions
    static const QString createRolePermissionsTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS role_permissions (
                role_id INTEGER,
                permission_id INTEGER,
                assigned_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                PRIMARY KEY (role_id, permission_id),
                FOREIGN KEY (role_id) REFERENCES roles(role_id) ON DELETE CASCADE,
                FOREIGN KEY (permission_id) REFERENCES permissions(permission_id) ON DELETE CASCADE
            );
        )";
    }

    // Tabel customers
    static const QString createCustomersTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS customers (
                customer_id INTEGER PRIMARY KEY,
                customer_name TEXT NOT NULL,
                email TEXT,
                phone_number TEXT,
                address TEXT,
                created_by INTEGER,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (created_by) REFERENCES users(user_id) ON DELETE SET NULL
            );
        )";
    }

    // Tabel products
    static const QString createProductsTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS products (
                product_id INTEGER PRIMARY KEY,
                product_name TEXT UNIQUE NOT NULL COLLATE NOCASE,
                description TEXT,
                default_price INTEGER NOT NULL DEFAULT 0,
                stock INTEGER NOT NULL DEFAULT 1000,  -- Jumlah stok yang tersedia
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP
            );
        )";
    }

    // Tabel orders
    static const QString createOrdersTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS orders (
                order_id INTEGER PRIMARY KEY,
                order_name TEXT NOT NULL,
                product_id INTEGER,
                order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                customer_id INTEGER,
                invoice_id INTEGER,  -- Menambahkan kolom untuk mengaitkan dengan tabel invoices
                quantity INTEGER NOT NULL,
                cost_production INTEGER NOT NULL,
                selling_price INTEGER NOT NULL,
                discount_amount INTEGER DEFAULT 0,  -- Diskon dalam satuan terkecil per order
                discount_percentage INTEGER DEFAULT 0, -- Diskon dalam persen per order
                length_mm INTEGER,    -- Panjang (opsional untuk barang yang memerlukan luas)
                width_mm INTEGER,     -- Lebar (opsional untuk barang yang memerlukan luas)
                total_price REAL AS 
                    (CASE 
                        WHEN length_mm IS NOT NULL AND width_mm IS NOT NULL THEN 
                            quantity * selling_price * (length_mm / 1000.0) * (width_mm / 1000.0) 
                            - discount_amount 
                        ELSE 
                            quantity * selling_price - discount_amount 
                    END) STORED,
                FOREIGN KEY (customer_id) REFERENCES customers(customer_id) ON DELETE SET NULL,
                FOREIGN KEY (invoice_id) REFERENCES invoices(invoice_id) ON DELETE CASCADE,  -- Menambahkan foreign key ke tabel invoices
                FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE SET NULL
            );
        )";
    }

    // Tabel invoices
    static const QString createInvoicesTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS invoices (
                user_id INTEGER NOT NULL, -- User pembuat invoice
                invoice_id INTEGER PRIMARY KEY,
                customer_id INTEGER,
                invoice_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                due_date TIMESTAMP, -- Menambahkan kolom untuk tanggal jatuh tempo
                total_amount INTEGER NOT NULL,
                amount_paid INTEGER DEFAULT 0,
                amount_remaining INTEGER AS (total_amount - amount_paid), -- Menghitung sisa pembayaran
                discount_amount INTEGER DEFAULT 0,  -- Diskon dalam satuan terkecil per invoice
                discount_percentage INTEGER DEFAULT 0, -- Diskon dalam persen per invoice
                payment_method TEXT CHECK(payment_method IN ('cash', 'transfer')),
                status TEXT CHECK(status IN ('pending', 'paid', 'partially_paid', 'overdue')) DEFAULT 'pending', -- Menambahkan status overdue
                FOREIGN KEY (customer_id) REFERENCES customers(customer_id) ON DELETE SET NULL,
                FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
            );
        )";
    }

    // Tabel invoice_payments
    static const QString createInvoicePaymentsTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS invoice_payments (
                payment_id INTEGER PRIMARY KEY,
                invoice_id INTEGER,
                payment_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                payment_modified_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                amount INTEGER NOT NULL,
                method TEXT CHECK(method IN ('cash', 'transfer')),
                note TEXT,  -- Catatan tambahan untuk pembayaran
                FOREIGN KEY (invoice_id) REFERENCES invoices(invoice_id) ON DELETE CASCADE
            );
        )";
    }

    // Tabel divisions
    static const QString createDivisionsTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS divisions (
                division_id INTEGER PRIMARY KEY,
                division_name TEXT UNIQUE NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )";
    }

    // Tabel expenses (Pengeluaran)
    static const QString createExpensesTable() {
        return R"(
            CREATE TABLE IF NOT EXISTS expenses (
                expense_id INTEGER PRIMARY KEY,
                division_id INTEGER, -- Mengaitkan dengan tabel divisions
                user_id INTEGER, -- Mengaitkan dengan tabel users untuk penanggung jawab
                expense_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                amount INTEGER NOT NULL, -- Jumlah pengeluaran
                description TEXT, -- Deskripsi pengeluaran
                category TEXT, -- Kategori pengeluaran (misalnya, transportasi, makanan, dll.)
                FOREIGN KEY (division_id) REFERENCES divisions(division_id) ON DELETE SET NULL,
                FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL -- Menambahkan foreign key ke tabel users
            );
        )";
    }
};

#endif // DATABASEQUERIES_H
