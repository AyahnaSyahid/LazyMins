--
-- File generated with SQLiteStudio v3.4.4 on Sen Des 16 08:44:24 2024
--
-- Text encoding used: UTF-8
--
PRAGMA foreign_keys = off;
BEGIN TRANSACTION;

-- Table: customers
CREATE TABLE IF NOT EXISTS customers (
    customer_id   INTEGER   PRIMARY KEY,
    customer_name TEXT      NOT NULL
                            UNIQUE,
    email         TEXT,
    phone_number  TEXT,
    address       TEXT,
    created_by    INTEGER,
    created_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (
        created_by
    )
    REFERENCES users (user_id) ON DELETE SET NULL
);


-- Table: divisions
CREATE TABLE IF NOT EXISTS divisions (
    division_id   INTEGER   PRIMARY KEY,
    division_name TEXT      UNIQUE
                            NOT NULL
                            COLLATE NOCASE,
    created_date  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


-- Table: expenses
CREATE TABLE IF NOT EXISTS expenses (
    expense_id    INTEGER   PRIMARY KEY,
    division_id   INTEGER   NOT NULL,
    expense_date  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                            NOT NULL,
    amount        INTEGER   NOT NULL,
    description   TEXT,
    category      TEXT,
    created_by    INTEGER,
    created_date  TIMESTAMP DEFAULT (CURRENT_TIMESTAMP),
    modified_by   INTEGER,
    modified_date TIMESTAMP,
    FOREIGN KEY (
        division_id
    )
    REFERENCES divisions (division_id) ON DELETE SET NULL
                                       ON UPDATE CASCADE,
    FOREIGN KEY (
        created_by
    )
    REFERENCES users (user_id) ON DELETE SET NULL
                               ON UPDATE CASCADE,
    FOREIGN KEY (
        modified_by
    )
    REFERENCES users (user_id) ON DELETE SET NULL
                               ON UPDATE CASCADE
);


-- Table: invoice_payments
CREATE TABLE IF NOT EXISTS invoice_payments (
    payment_id    INTEGER   PRIMARY KEY,
    payment_date  TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                            NOT NULL,
    invoice_id    INTEGER   NOT NULL,
    amount        INTEGER   NOT NULL,
    method        TEXT      CHECK (method IN ('cash', 'transfer') ) 
                            NOT NULL,
    note          TEXT,
    created_by    INTEGER,
    created_date  TIMESTAMP DEFAULT (CURRENT_TIMESTAMP),
    modified_by   INTEGER,
    modified_date TIMESTAMP,
    FOREIGN KEY (
        invoice_id
    )
    REFERENCES invoices (invoice_id) ON DELETE CASCADE,
    FOREIGN KEY (
        created_by
    )
    REFERENCES users (user_id) ON DELETE SET NULL,
    FOREIGN KEY (
        modified_by
    )
    REFERENCES users (user_id) ON DELETE SET NULL
);


-- Table: invoices
CREATE TABLE IF NOT EXISTS invoices (
    invoice_id      INTEGER   PRIMARY KEY,
    physical_iid    TEXT,
    user_id         INTEGER   NOT NULL,
    customer_id     INTEGER,
    invoice_date    TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    due_date        TIMESTAMP,
    total_amount    INTEGER,
    total_paid      INTEGER,
    discount_amount INTEGER,
    created_date    TIMESTAMP,
    modified_date   TIMESTAMP,
    modified_by     INTEGER,
    FOREIGN KEY (
        customer_id
    )
    REFERENCES customers (customer_id) ON DELETE SET NULL
                                       ON UPDATE CASCADE,
    FOREIGN KEY (
        user_id
    )
    REFERENCES users (user_id) ON DELETE SET NULL,
    FOREIGN KEY (
        modified_by
    )
    REFERENCES users (user_id) 
);


-- Table: orders
CREATE TABLE IF NOT EXISTS orders (
    order_id        INTEGER   PRIMARY KEY,
    order_name      TEXT      NOT NULL,
    product_id      INTEGER,
    order_date      TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                              NOT NULL,
    customer_id     INTEGER,
    invoice_id      INTEGER,
    quantity        INTEGER   NOT NULL,
    cost_production INTEGER,
    selling_price   INTEGER   NOT NULL,
    discount_amount INTEGER   DEFAULT 0,
    width_mm        INTEGER,
    height_mm       INTEGER,
    total_price     [REAL ]   GENERATED ALWAYS AS (CAST ( (quantity * selling_price * CASE WHEN height_mm IS NOT NULL AND 
                                                                                                width_mm IS NOT NULL THEN (height_mm / 1000.0) * (width_mm / 1000.0) ELSE 1 END - discount_amount + 99) / 100 AS INTEGER) * 100) STORED,
    created_by      INTEGER,
    created_date    TIMESTAMP NOT NULL
                              DEFAULT (CURRENT_TIMESTAMP),
    modified_by     INTEGER,
    modified_date   TIMESTAMP,
    FOREIGN KEY (
        product_id
    )
    REFERENCES products (product_id),
    FOREIGN KEY (
        invoice_id
    )
    REFERENCES invoices (invoice_id) ON DELETE SET NULL
                                     ON UPDATE CASCADE,
    FOREIGN KEY (
        customer_id
    )
    REFERENCES customers (customer_id),
    FOREIGN KEY (
        modified_by
    )
    REFERENCES users (user_id),
    FOREIGN KEY (
        created_by
    )
    REFERENCES users (user_id),
    CONSTRAINT TidakRugi CHECK (selling_price >= cost_production),
    CHECK (quantity >= 1) 
);


-- Table: permissions
CREATE TABLE IF NOT EXISTS permissions (
    permission_id   INTEGER PRIMARY KEY,
    permission_name TEXT    UNIQUE
                            NOT NULL,
    description     TEXT
);


-- Table: products
CREATE TABLE IF NOT EXISTS products (
    product_id  INTEGER   PRIMARY KEY,
    code        TEXT      UNIQUE
                          NOT NULL
                          COLLATE NOCASE,
    name        TEXT,
    qr_bar      TEXT      UNIQUE,
    cost        INTEGER,
    price       INTEGER   DEFAULT 0,
    use_area    BOOL      DEFAULT (0),
    description TEXT,
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at  TIMESTAMP
);


-- Table: role_permissions
CREATE TABLE IF NOT EXISTS role_permissions (
    role_id       INTEGER,
    permission_id INTEGER,
    assigned_at   TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (
        role_id,
        permission_id
    ),
    FOREIGN KEY (
        role_id
    )
    REFERENCES roles (role_id) ON DELETE CASCADE,
    FOREIGN KEY (
        permission_id
    )
    REFERENCES permissions (permission_id) ON DELETE CASCADE
);


-- Table: roles
CREATE TABLE IF NOT EXISTS roles (
    role_id     INTEGER PRIMARY KEY,
    role_name   TEXT    UNIQUE
                        NOT NULL,
    description TEXT
);


-- Table: user_roles
CREATE TABLE IF NOT EXISTS user_roles (
    user_id     INTEGER,
    role_id     INTEGER,
    assigned_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (
        user_id,
        role_id
    ),
    FOREIGN KEY (
        user_id
    )
    REFERENCES users (user_id) ON DELETE CASCADE,
    FOREIGN KEY (
        role_id
    )
    REFERENCES roles (role_id) ON DELETE CASCADE
);


-- Table: users
CREATE TABLE IF NOT EXISTS users (
    user_id       INTEGER   PRIMARY KEY,
    username      TEXT      UNIQUE
                            NOT NULL,
    salt          TEXT      NOT NULL,
    password_hash TEXT      NOT NULL,
    full_name     TEXT,
    email         TEXT,
    phone_number  TEXT,
    home_address  TEXT,
    is_active     BOOLEAN   DEFAULT 1,
    last_active   TIMESTAMP,
    created_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


COMMIT TRANSACTION;
PRAGMA foreign_keys = on;
