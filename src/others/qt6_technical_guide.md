# 🔧 Panduan Teknis Qt6 C++ untuk Aplikasi POS Percetakan

## 📚 Implementasi Pattern & Best Practices

### 1. Repository Pattern untuk Database Access

```cpp
// src/repositories/BaseRepository.h
#ifndef BASEREPOSITORY_H
#define BASEREPOSITORY_H

#include <QSqlQuery>
#include <QSqlError>
#include <QVariantList>
#include <QString>
#include "database/DatabaseManager.h"

template<typename T>
class BaseRepository
{
public:
    virtual ~BaseRepository() = default;
    
    // CRUD operations
    virtual bool create(T* model) = 0;
    virtual T* findById(int id) = 0;
    virtual QList<T*> findAll() = 0;
    virtual bool update(T* model) = 0;
    virtual bool remove(int id) = 0;
    
    // Pagination
    virtual QList<T*> paginate(int page, int perPage) = 0;
    virtual int count() = 0;
    
protected:
    QSqlDatabase& db() {
        return DatabaseManager::instance().database();
    }
    
    QString lastError() const {
        return m_lastError;
    }
    
    void setLastError(const QString& error) {
        m_lastError = error;
    }
    
private:
    QString m_lastError;
};

#endif
```

```cpp
// src/repositories/CustomerRepository.h
#ifndef CUSTOMERREPOSITORY_H
#define CUSTOMERREPOSITORY_H

#include "BaseRepository.h"
#include "models/Customer.h"
#include <QList>

class CustomerRepository : public BaseRepository<Customer>
{
public:
    CustomerRepository();
    
    // Implement base methods
    bool create(Customer* customer) override;
    Customer* findById(int id) override;
    QList<Customer*> findAll() override;
    bool update(Customer* customer) override;
    bool remove(int id) override;
    
    QList<Customer*> paginate(int page, int perPage) override;
    int count() override;
    
    // Custom queries
    Customer* findByCode(const QString& customerCode);
    QList<Customer*> findByName(const QString& name);
    QList<Customer*> findByPhone(const QString& phone);
    QList<Customer*> findActive();
    QList<Customer*> findTopCustomers(int limit = 10);
    
    // Statistics
    double getTotalRevenue(int customerId);
    int getTotalOrders(int customerId);
    
private:
    Customer* mapToModel(const QSqlQuery& query);
};

#endif
```

```cpp
// src/repositories/CustomerRepository.cpp
#include "CustomerRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

CustomerRepository::CustomerRepository()
{
}

bool CustomerRepository::create(Customer* customer)
{
    if (!customer || !customer->isValid()) {
        setLastError("Invalid customer data");
        return false;
    }
    
    QSqlQuery query(db());
    query.prepare(
        "INSERT INTO konsumen (nama_lengkap, customer_type, email, nomor_telp, "
        "alamat, kota, kode_pos, price_level_id, is_active) "
        "VALUES (:nama, :type, :email, :telp, :alamat, :kota, :pos, :price_level, :active)"
    );
    
    query.bindValue(":nama", customer->namaLengkap());
    query.bindValue(":type", customer->customerType());
    query.bindValue(":email", customer->email());
    query.bindValue(":telp", customer->nomorTelp());
    query.bindValue(":alamat", customer->alamat());
    query.bindValue(":kota", customer->kota());
    query.bindValue(":pos", customer->kodePos());
    query.bindValue(":price_level", customer->priceLevelId());
    query.bindValue(":active", customer->isActive() ? 1 : 0);
    
    if (!query.exec()) {
        setLastError(query.lastError().text());
        qCritical() << "Failed to insert customer:" << lastError();
        return false;
    }
    
    customer->setId(query.lastInsertId().toInt());
    return true;
}

Customer* CustomerRepository::findById(int id)
{
    QSqlQuery query(db());
    query.prepare("SELECT * FROM konsumen WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec() || !query.next()) {
        setLastError(query.lastError().text());
        return nullptr;
    }
    
    return mapToModel(query);
}

QList<Customer*> CustomerRepository::findAll()
{
    QList<Customer*> customers;
    
    QSqlQuery query(db());
    if (!query.exec("SELECT * FROM konsumen ORDER BY nama_lengkap")) {
        setLastError(query.lastError().text());
        return customers;
    }
    
    while (query.next()) {
        customers.append(mapToModel(query));
    }
    
    return customers;
}

bool CustomerRepository::update(Customer* customer)
{
    if (!customer || !customer->isValid() || customer->id() <= 0) {
        setLastError("Invalid customer data");
        return false;
    }
    
    QSqlQuery query(db());
    query.prepare(
        "UPDATE konsumen SET "
        "nama_lengkap = :nama, customer_type = :type, email = :email, "
        "nomor_telp = :telp, alamat = :alamat, kota = :kota, kode_pos = :pos, "
        "price_level_id = :price_level, is_active = :active "
        "WHERE id = :id"
    );
    
    query.bindValue(":id", customer->id());
    query.bindValue(":nama", customer->namaLengkap());
    query.bindValue(":type", customer->customerType());
    query.bindValue(":email", customer->email());
    query.bindValue(":telp", customer->nomorTelp());
    query.bindValue(":alamat", customer->alamat());
    query.bindValue(":kota", customer->kota());
    query.bindValue(":pos", customer->kodePos());
    query.bindValue(":price_level", customer->priceLevelId());
    query.bindValue(":active", customer->isActive() ? 1 : 0);
    
    if (!query.exec()) {
        setLastError(query.lastError().text());
        return false;
    }
    
    return true;
}

bool CustomerRepository::remove(int id)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM konsumen WHERE id = :id");
    query.bindValue(":id", id);
    
    if (!query.exec()) {
        setLastError(query.lastError().text());
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

QList<Customer*> CustomerRepository::paginate(int page, int perPage)
{
    QList<Customer*> customers;
    int offset = (page - 1) * perPage;
    
    QSqlQuery query(db());
    query.prepare(
        "SELECT * FROM konsumen "
        "ORDER BY nama_lengkap "
        "LIMIT :limit OFFSET :offset"
    );
    query.bindValue(":limit", perPage);
    query.bindValue(":offset", offset);
    
    if (!query.exec()) {
        setLastError(query.lastError().text());
        return customers;
    }
    
    while (query.next()) {
        customers.append(mapToModel(query));
    }
    
    return customers;
}

int CustomerRepository::count()
{
    QSqlQuery query(db());
    if (query.exec("SELECT COUNT(*) FROM konsumen") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

Customer* CustomerRepository::findByCode(const QString& customerCode)
{
    QSqlQuery query(db());
    query.prepare("SELECT * FROM konsumen WHERE customer_code = :code");
    query.bindValue(":code", customerCode);
    
    if (!query.exec() || !query.next()) {
        return nullptr;
    }
    
    return mapToModel(query);
}

QList<Customer*> CustomerRepository::findByName(const QString& name)
{
    QList<Customer*> customers;
    
    QSqlQuery query(db());
    query.prepare(
        "SELECT * FROM konsumen "
        "WHERE nama_lengkap LIKE :name "
        "ORDER BY nama_lengkap"
    );
    query.bindValue(":name", "%" + name + "%");
    
    if (!query.exec()) {
        return customers;
    }
    
    while (query.next()) {
        customers.append(mapToModel(query));
    }
    
    return customers;
}

QList<Customer*> CustomerRepository::findActive()
{
    QList<Customer*> customers;
    
    QSqlQuery query(db());
    if (!query.exec("SELECT * FROM konsumen WHERE is_active = 1 ORDER BY nama_lengkap")) {
        return customers;
    }
    
    while (query.next()) {
        customers.append(mapToModel(query));
    }
    
    return customers;
}

Customer* CustomerRepository::mapToModel(const QSqlQuery& query)
{
    Customer* customer = new Customer();
    customer->setId(query.value("id").toInt());
    customer->setCustomerCode(query.value("customer_code").toString());
    customer->setNamaLengkap(query.value("nama_lengkap").toString());
    customer->setCustomerType(query.value("customer_type").toString());
    customer->setEmail(query.value("email").toString());
    customer->setNomorTelp(query.value("nomor_telp").toString());
    customer->setAlamat(query.value("alamat").toString());
    customer->setKota(query.value("kota").toString());
    customer->setKodePos(query.value("kode_pos").toString());
    customer->setPriceLevelId(query.value("price_level_id").toInt());
    customer->setIsActive(query.value("is_active").toBool());
    customer->setTotalOrders(query.value("total_orders").toInt());
    customer->setTotalSpent(query.value("total_spent").toDouble());
    customer->setCreatedAt(query.value("created_at").toDateTime());
    customer->setUpdatedAt(query.value("updated_at").toDateTime());
    
    return customer;
}
```

---

### 2. Model/View Programming dengan QTableView

```cpp
// src/views/customers/CustomerListWidget.h
#ifndef CUSTOMERLISTWIDGET_H
#define CUSTOMERLISTWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLineEdit>
#include "models/Customer.h"
#include "repositories/CustomerRepository.h"

namespace Ui {
class CustomerListWidget;
}

class CustomerListWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit CustomerListWidget(QWidget *parent = nullptr);
    ~CustomerListWidget();
    
public slots:
    void refreshList();
    void onSearchTextChanged(const QString& text);
    void onAddCustomer();
    void onEditCustomer();
    void onDeleteCustomer();
    void onViewDetails();
    
private:
    Ui::CustomerListWidget *ui;
    QStandardItemModel *m_model;
    CustomerRepository m_repository;
    
    void setupTable();
    void setupConnections();
    void loadCustomers(const QList<Customer*>& customers);
    Customer* getSelectedCustomer();
};

#endif
```

```cpp
// src/views/customers/CustomerListWidget.cpp
#include "CustomerListWidget.h"
#include "ui_CustomerListWidget.h"
#include "CustomerFormDialog.h"
#include <QMessageBox>
#include <QHeaderView>

CustomerListWidget::CustomerListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomerListWidget)
    , m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    setupTable();
    setupConnections();
    refreshList();
}

CustomerListWidget::~CustomerListWidget()
{
    delete ui;
}

void CustomerListWidget::setupTable()
{
    // Setup model
    m_model->setHorizontalHeaderLabels({
        "ID", "Kode", "Nama", "Tipe", "Telepon", 
        "Email", "Kota", "Total Order", "Total Belanja", "Status"
    });
    
    ui->tableView->setModel(m_model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    
    // Hide ID column
    ui->tableView->setColumnHidden(0, true);
    
    // Set column widths
    ui->tableView->setColumnWidth(1, 100);  // Kode
    ui->tableView->setColumnWidth(2, 200);  // Nama
    ui->tableView->setColumnWidth(3, 100);  // Tipe
    ui->tableView->setColumnWidth(4, 120);  // Telepon
    ui->tableView->setColumnWidth(5, 150);  // Email
}

void CustomerListWidget::setupConnections()
{
    connect(ui->btnAdd, &QPushButton::clicked, this, &CustomerListWidget::onAddCustomer);
    connect(ui->btnEdit, &QPushButton::clicked, this, &CustomerListWidget::onEditCustomer);
    connect(ui->btnDelete, &QPushButton::clicked, this, &CustomerListWidget::onDeleteCustomer);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &CustomerListWidget::refreshList);
    connect(ui->searchBox, &QLineEdit::textChanged, this, &CustomerListWidget::onSearchTextChanged);
    connect(ui->tableView, &QTableView::doubleClicked, this, &CustomerListWidget::onViewDetails);
}

void CustomerListWidget::refreshList()
{
    QList<Customer*> customers = m_repository.findAll();
    loadCustomers(customers);
    
    // Clean up
    qDeleteAll(customers);
}

void CustomerListWidget::loadCustomers(const QList<Customer*>& customers)
{
    m_model->removeRows(0, m_model->rowCount());
    
    for (Customer* customer : customers) {
        QList<QStandardItem*> row;
        
        row << new QStandardItem(QString::number(customer->id()));
        row << new QStandardItem(customer->customerCode());
        row << new QStandardItem(customer->namaLengkap());
        row << new QStandardItem(customer->customerType());
        row << new QStandardItem(customer->nomorTelp());
        row << new QStandardItem(customer->email());
        row << new QStandardItem(customer->kota());
        row << new QStandardItem(QString::number(customer->totalOrders()));
        
        // Format currency
        QString totalSpent = QString("Rp %L1")
            .arg(customer->totalSpent(), 0, 'f', 0);
        row << new QStandardItem(totalSpent);
        
        // Status
        QString status = customer->isActive() ? "Aktif" : "Non-Aktif";
        QStandardItem* statusItem = new QStandardItem(status);
        if (customer->isActive()) {
            statusItem->setForeground(QBrush(Qt::darkGreen));
        } else {
            statusItem->setForeground(QBrush(Qt::red));
        }
        row << statusItem;
        
        m_model->appendRow(row);
    }
}

void CustomerListWidget::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        refreshList();
        return;
    }
    
    QList<Customer*> customers = m_repository.findByName(text);
    loadCustomers(customers);
    qDeleteAll(customers);
}

void CustomerListWidget::onAddCustomer()
{
    CustomerFormDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshList();
    }
}

void CustomerListWidget::onEditCustomer()
{
    Customer* customer = getSelectedCustomer();
    if (!customer) {
        QMessageBox::warning(this, "Peringatan", "Pilih customer terlebih dahulu");
        return;
    }
    
    CustomerFormDialog dialog(customer, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshList();
    }
    
    delete customer;
}

void CustomerListWidget::onDeleteCustomer()
{
    Customer* customer = getSelectedCustomer();
    if (!customer) {
        QMessageBox::warning(this, "Peringatan", "Pilih customer terlebih dahulu");
        return;
    }
    
    int ret = QMessageBox::question(
        this, 
        "Konfirmasi", 
        QString("Hapus customer '%1'?").arg(customer->namaLengkap()),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (ret == QMessageBox::Yes) {
        if (m_repository.remove(customer->id())) {
            QMessageBox::information(this, "Sukses", "Customer berhasil dihapus");
            refreshList();
        } else {
            QMessageBox::critical(this, "Error", 
                "Gagal menghapus customer: " + m_repository.lastError());
        }
    }
    
    delete customer;
}

Customer* CustomerListWidget::getSelectedCustomer()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return nullptr;
    }
    
    int row = selected.first().row();
    int customerId = m_model->item(row, 0)->text().toInt();
    
    return m_repository.findById(customerId);
}
```

---

### 3. Order Entry dengan Real-time Calculation

```cpp
// src/controllers/OrderController.h
#ifndef ORDERCONTROLLER_H
#define ORDERCONTROLLER_H

#include <QObject>
#include "models/Order.h"
#include "models/OrderItem.h"
#include "models/Product.h"
#include "repositories/OrderRepository.h"
#include "repositories/ProductRepository.h"

class OrderController : public QObject
{
    Q_OBJECT
    
public:
    explicit OrderController(QObject *parent = nullptr);
    
    // Order operations
    bool createOrder(Order* order, const QList<OrderItem*>& items);
    bool updateOrder(Order* order, const QList<OrderItem*>& items);
    bool cancelOrder(int orderId);
    
    // Calculations
    struct OrderCalculation {
        double subtotal;
        double discountAmount;
        double taxAmount;
        double totalAmount;
    };
    
    OrderCalculation calculateOrder(
        const QList<OrderItem*>& items,
        double discountPercentage = 0.0
    );
    
    // Order number generation
    QString generateOrderNumber();
    
signals:
    void orderCreated(int orderId);
    void orderUpdated(int orderId);
    void orderCancelled(int orderId);
    void errorOccurred(const QString& error);
    
private:
    OrderRepository m_orderRepository;
    ProductRepository m_productRepository;
    
    bool validateOrder(Order* order, const QList<OrderItem*>& items);
    bool checkStockAvailability(const QList<OrderItem*>& items);
};

#endif
```

```cpp
// src/controllers/OrderController.cpp
#include "OrderController.h"
#include "database/DatabaseManager.h"
#include <QDateTime>
#include <QDebug>

OrderController::OrderController(QObject *parent)
    : QObject(parent)
{
}

bool OrderController::createOrder(Order* order, const QList<OrderItem*>& items)
{
    if (!validateOrder(order, items)) {
        return false;
    }
    
    if (!checkStockAvailability(items)) {
        emit errorOccurred("Stok tidak mencukupi untuk beberapa produk");
        return false;
    }
    
    // Calculate totals
    auto calc = calculateOrder(items, order->discountPercentage());
    order->setSubtotal(calc.subtotal);
    order->setDiscountAmount(calc.discountAmount);
    order->setTaxAmount(calc.taxAmount);
    order->setTotalAmount(calc.totalAmount);
    
    // Generate order number
    order->setOrderNumber(generateOrderNumber());
    
    // Begin transaction
    auto& db = DatabaseManager::instance();
    if (!db.beginTransaction()) {
        emit errorOccurred("Gagal memulai transaksi database");
        return false;
    }
    
    // Save order
    if (!m_orderRepository.create(order)) {
        db.rollback();
        emit errorOccurred("Gagal menyimpan order: " + m_orderRepository.lastError());
        return false;
    }
    
    // Save order items
    for (OrderItem* item : items) {
        item->setOrderId(order->id());
        if (!m_orderRepository.createOrderItem(item)) {
            db.rollback();
            emit errorOccurred("Gagal menyimpan item order");
            return false;
        }
    }
    
    // Commit transaction
    if (!db.commit()) {
        db.rollback();
        emit errorOccurred("Gagal commit transaksi");
        return false;
    }
    
    emit orderCreated(order->id());
    return true;
}

OrderController::OrderCalculation OrderController::calculateOrder(
    const QList<OrderItem*>& items, 
    double discountPercentage)
{
    OrderCalculation calc;
    calc.subtotal = 0.0;
    
    // Calculate subtotal
    for (const OrderItem* item : items) {
        calc.subtotal += item->subtotal();
    }
    
    // Calculate discount
    calc.discountAmount = calc.subtotal * (discountPercentage / 100.0);
    
    // Calculate tax (PPN 11%)
    double afterDiscount = calc.subtotal - calc.discountAmount;
    calc.taxAmount = afterDiscount * 0.11;
    
    // Calculate total
    calc.totalAmount = afterDiscount + calc.taxAmount;
    
    return calc;
}

QString OrderController::generateOrderNumber()
{
    QDateTime now = QDateTime::currentDateTime();
    QString date = now.toString("yyyyMMdd");
    
    // Get today's order count
    int count = m_orderRepository.getTodayOrderCount();
    
    return QString("ORD-%1-%2")
        .arg(date)
        .arg(count + 1, 4, 10, QChar('0'));
}

bool OrderController::validateOrder(Order* order, const QList<OrderItem*>& items)
{
    if (items.isEmpty()) {
        emit errorOccurred("Order harus memiliki minimal 1 item");
        return false;
    }
    
    if (!order->isValid()) {
        emit errorOccurred("Data order tidak valid");
        return false;
    }
    
    return true;
}

bool OrderController::checkStockAvailability(const QList<OrderItem*>& items)
{
    for (const OrderItem* item : items) {
        if (item->productId() > 0) {
            Product* product = m_productRepository.findById(item->productId());
            if (!product) {
                return false;
            }
            
            if (product->stock() < item->quantity()) {
                delete product;
                return false;
            }
            
            delete product;
        }
    }
    
    return true;
}
```

---

### 4. Utility Classes

```cpp
// src/utils/NumberFormatter.h
#ifndef NUMBERFORMATTER_H
#define NUMBERFORMATTER_H

#include <QString>
#include <QLocale>

class NumberFormatter
{
public:
    // Format as Indonesian Rupiah
    static QString formatCurrency(double amount) {
        QLocale indonesian(QLocale::Indonesian, QLocale::Indonesia);
        return QString("Rp %1").arg(
            indonesian.toString(amount, 'f', 0)
        );
    }
    
    // Format as percentage
    static QString formatPercentage(double value) {
        return QString("%1%").arg(value, 0, 'f', 2);
    }
    
    // Format as number with thousand separator
    static QString formatNumber(double value, int decimals = 0) {
        QLocale indonesian(QLocale::Indonesian, QLocale::Indonesia);
        return indonesian.toString(value, 'f', decimals);
    }
    
    // Parse currency string to double
    static double parseCurrency(const QString& str) {
        QString cleaned = str;
        cleaned.remove("Rp").remove(".").remove(",").trimmed();
        return cleaned.toDouble();
    }
};

#endif
```

```cpp
// src/utils/Validator.h
#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <QString>
#include <QRegularExpression>

class Validator
{
public:
    // Email validation
    static bool isValidEmail(const QString& email) {
        if (email.isEmpty()) return true; // Email is optional
        
        QRegularExpression regex(
            R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
        );
        return regex.match(email).hasMatch();
    }
    
    // Phone number validation (Indonesian format)
    static bool isValidPhoneNumber(const QString& phone) {
        if (phone.isEmpty()) return true;
        
        QRegularExpression regex(R"(^(\+62|62|0)[0-9]{9,12}$)");
        return regex.match(phone).hasMatch();
    }
    
    // Check if string is not empty
    static bool isNotEmpty(const QString& str) {
        return !str.trimmed().isEmpty();
    }
    
    // Check if number is positive
    static bool isPositive(double value) {
        return value > 0;
    }
    
    // Check if number is non-negative
    static bool isNonNegative(double value) {
        return value >= 0;
    }
    
    // Validate Indonesian postal code
    static bool isValidPostalCode(const QString& code) {
        if (code.isEmpty()) return true;
        
        QRegularExpression regex(R"(^\d{5}$)");
        return regex.match(code).hasMatch();
    }
};

#endif
```

---

### 5. Custom Widgets

```cpp
// src/widgets/SearchBox.h
#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QLineEdit>
#include <QTimer>

class SearchBox : public QLineEdit
{
    Q_OBJECT
    
public:
    explicit SearchBox(QWidget *parent = nullptr);
    
    void setSearchDelay(int milliseconds);
    
signals:
    void searchTriggered(const QString& text);
    
private slots:
    void onTextChanged(const QString& text);
    void onSearchTimeout();
    
private:
    QTimer* m_searchTimer;
    int m_searchDelay;
};

#endif
```

```cpp
// src/widgets/SearchBox.cpp
#include "SearchBox.h"

SearchBox::SearchBox(QWidget *parent)
    : QLineEdit(parent)
    , m_searchTimer(new QTimer(this))
    , m_searchDelay(300)
{
    setPlaceholderText("Cari...");
    setClearButtonEnabled(true);
    
    m_searchTimer->setSingleShot(true);
    
    connect(this, &QLineEdit::textChanged, 
            this, &SearchBox::onTextChanged);
    connect(m_searchTimer, &QTimer::timeout, 
            this, &SearchBox::onSearchTimeout);
}

void SearchBox::setSearchDelay(int milliseconds)
{
    m_searchDelay = milliseconds;
}

void SearchBox::onTextChanged(const QString& text)
{
    m_searchTimer->stop();
    m_searchTimer->start(m_searchDelay);
}

void SearchBox::onSearchTimeout()
{
    emit searchTriggered(text());
}
```

---

### 6. Report Generation dengan QPrinter

```cpp
// src/utils/ReportGenerator.h
#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QPrinter>
#include <QPainter>
#include <QString>
#include "models/Order.h"

class ReportGenerator
{
public:
    // Print order receipt
    static bool printOrderReceipt(Order* order, const QString& printerName = QString());
    
    // Generate PDF receipt
    static bool generatePdfReceipt(Order* order, const QString& filePath);
    
    // Print sales report
    static bool printSalesReport(
        const QDate& startDate,
        const QDate& endDate,
        const QString& printerName = QString()
    );
    
private:
    static void drawHeader(QPainter& painter, const QRectF& rect);
    static void drawOrderDetails(QPainter& painter, Order* order, const QRectF& rect);
    static void drawFooter(QPainter& painter, const QRectF& rect);
    
    static constexpr int PAGE_WIDTH = 80;  // mm (thermal printer)
    static constexpr int MARGIN = 5;       // mm
};

#endif
```

---

### 7. Settings Management

```cpp
// src/utils/AppSettings.h
#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QString>

class AppSettings
{
public:
    static AppSettings& instance() {
        static AppSettings instance;
        return instance;
    }
    
    // Company info
    QString companyName() const;
    void setCompanyName(const QString& name);
    
    QString companyAddress() const;
    void setCompanyAddress(const QString& address);
    
    QString companyPhone() const;
    void setCompanyPhone(const QString& phone);
    
    // Tax settings
    double taxRate() const;
    void setTaxRate(double rate);
    
    // Printer settings
    QString defaultPrinter() const;
    void setDefaultPrinter(const QString& printer);
    
    // UI settings
    QString theme() const;
    void setTheme(const QString& theme);
    
    // Backup settings
    QString backupDirectory() const;
    void setBackupDirectory(const QString& dir);
    
private:
    AppSettings();
    QSettings m_settings;
};

#endif
```

---

## 🎯 Tips Performance Optimization

### 1. Database Query Optimization

```cpp
// Use prepared statements
QSqlQuery query;
query.prepare("SELECT * FROM orders WHERE customer_id = ? AND status = ?");
query.addBindValue(customerId);
query.addBindValue(status);
query.exec();

// Batch insert
db.transaction();
for (const auto& item : items) {
    query.exec(); // insert query
}
db.commit();

// Use indexes (already in schema)
// Avoid SELECT * in production
```

### 2. Model/View Caching

```cpp
// Cache frequently accessed data
class ProductCache {
private:
    QMap<int, Product*> m_cache;
    QDateTime m_lastUpdate;
    
public:
    Product* get(int id) {
        if (shouldRefresh()) refresh();
        return m_cache.value(id);
    }
};
```

### 3. Lazy Loading Images

```cpp
// Load images on demand, not all at once
void ProductListWidget::onImageRequested(const QModelIndex& index) {
    if (!m_imageCache.contains(index.row())) {
        // Load image asynchronously
        loadImageAsync(index.row());
    }
}
```

---

## 🔐 Security Best Practices

### 1. Password Hashing

```cpp
#include <QCryptographicHash>
#include <QRandomGenerator>

QString hashPassword(const QString& password, const QString& salt) {
    QString salted = password + salt;
    QByteArray hash = QCryptographicHash::hash(
        salted.toUtf8(),
        QCryptographicHash::Sha256
    );
    return hash.toHex();
}

QString generateSalt() {
    QByteArray salt;
    for (int i = 0; i < 16; ++i) {
        salt.append(QRandomGenerator::global()->generate());
    }
    return salt.toHex();
}
```

### 2. SQL Injection Prevention

```cpp
// ALWAYS use prepared statements
// NEVER concatenate user input directly

// ❌ WRONG
QString query = "SELECT * FROM users WHERE username = '" + username + "'";

// ✅ CORRECT
QSqlQuery query;
query.prepare("SELECT * FROM users WHERE username = ?");
query.addBindValue(username);
```

---

## 📱 Responsive UI Tips

```cpp
// Use layouts, not fixed positions
QVBoxLayout* layout = new QVBoxLayout(this);
layout->addWidget(widget1);
layout->addWidget(widget2);

// Use size policies
widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

// Use splitters for resizable sections
QSplitter* splitter = new QSplitter(Qt::Horizontal);
splitter->addWidget(leftWidget);
splitter->addWidget(rightWidget);
```

---

**Gunakan contoh-contoh kode ini sebagai referensi saat development!**
