# 📁 Struktur Project Qt6 C++ - Aplikasi POS Percetakan

## Directory Structure

```
PercetakanPOS/
│
├── CMakeLists.txt                 # Build configuration
├── README.md                       # Project documentation
├── .gitignore                      # Git ignore rules
│
├── src/                            # Source code
│   ├── main.cpp                    # Application entry point
│   │
│   ├── database/                   # Database layer
│   │   ├── DatabaseManager.h
│   │   ├── DatabaseManager.cpp
│   │   ├── DatabaseConfig.h
│   │   └── Migration.cpp
│   │
│   ├── models/                     # Data models
│   │   ├── BaseModel.h
│   │   ├── BaseModel.cpp
│   │   ├── Admin.h
│   │   ├── Admin.cpp
│   │   ├── Customer.h
│   │   ├── Customer.cpp
│   │   ├── Product.h
│   │   ├── Product.cpp
│   │   ├── Order.h
│   │   ├── Order.cpp
│   │   ├── OrderItem.h
│   │   ├── OrderItem.cpp
│   │   ├── Payment.h
│   │   ├── Payment.cpp
│   │   └── FinishingService.h
│   │
│   ├── repositories/               # Data access layer
│   │   ├── BaseRepository.h
│   │   ├── AdminRepository.h
│   │   ├── CustomerRepository.h
│   │   ├── ProductRepository.h
│   │   ├── OrderRepository.h
│   │   └── PaymentRepository.h
│   │
│   ├── controllers/                # Business logic
│   │   ├── AuthController.h
│   │   ├── AuthController.cpp
│   │   ├── OrderController.h
│   │   ├── OrderController.cpp
│   │   ├── PaymentController.h
│   │   └── StockController.h
│   │
│   ├── views/                      # UI components
│   │   ├── MainWindow.h
│   │   ├── MainWindow.cpp
│   │   ├── MainWindow.ui
│   │   │
│   │   ├── auth/
│   │   │   ├── LoginDialog.h
│   │   │   ├── LoginDialog.cpp
│   │   │   └── LoginDialog.ui
│   │   │
│   │   ├── dashboard/
│   │   │   ├── DashboardWidget.h
│   │   │   ├── DashboardWidget.cpp
│   │   │   └── DashboardWidget.ui
│   │   │
│   │   ├── customers/
│   │   │   ├── CustomerListWidget.h
│   │   │   ├── CustomerListWidget.cpp
│   │   │   ├── CustomerFormDialog.h
│   │   │   └── CustomerFormDialog.cpp
│   │   │
│   │   ├── products/
│   │   │   ├── ProductListWidget.h
│   │   │   ├── ProductListWidget.cpp
│   │   │   ├── ProductFormDialog.h
│   │   │   └── ProductFormDialog.cpp
│   │   │
│   │   ├── orders/
│   │   │   ├── OrderEntryWidget.h
│   │   │   ├── OrderEntryWidget.cpp
│   │   │   ├── OrderListWidget.h
│   │   │   ├── OrderListWidget.cpp
│   │   │   └── OrderDetailDialog.h
│   │   │
│   │   ├── payments/
│   │   │   ├── PaymentDialog.h
│   │   │   ├── PaymentDialog.cpp
│   │   │   └── CashierWidget.h
│   │   │
│   │   ├── reports/
│   │   │   ├── ReportWidget.h
│   │   │   ├── SalesReportWidget.h
│   │   │   └── StockReportWidget.h
│   │   │
│   │   └── settings/
│   │       ├── SettingsDialog.h
│   │       └── SettingsDialog.cpp
│   │
│   ├── utils/                      # Utility classes
│   │   ├── Logger.h
│   │   ├── Logger.cpp
│   │   ├── Validator.h
│   │   ├── Validator.cpp
│   │   ├── DateTimeHelper.h
│   │   ├── NumberFormatter.h
│   │   ├── PasswordHasher.h
│   │   └── PdfGenerator.h
│   │
│   └── widgets/                    # Custom widgets
│       ├── SearchBox.h
│       ├── SearchBox.cpp
│       ├── DateRangePicker.h
│       ├── NumberInput.h
│       └── StatusBadge.h
│
├── resources/                      # Resource files
│   ├── resources.qrc               # Qt resource file
│   ├── icons/                      # Application icons
│   ├── images/                     # Images
│   ├── styles/                     # QSS stylesheets
│   │   └── default.qss
│   └── database/
│       └── percetakan_schema.sql   # Database schema
│
├── tests/                          # Unit tests
│   ├── CMakeLists.txt
│   ├── test_models.cpp
│   ├── test_repositories.cpp
│   └── test_controllers.cpp
│
└── docs/                           # Documentation
    ├── user_manual.md
    ├── api_documentation.md
    └── screenshots/
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)

project(PercetakanPOS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Sql 
    Charts 
    PrintSupport
)

# Source files
set(SOURCES
    src/main.cpp
    src/database/DatabaseManager.cpp
    src/models/BaseModel.cpp
    src/models/Admin.cpp
    src/models/Customer.cpp
    src/models/Product.cpp
    src/models/Order.cpp
    src/controllers/AuthController.cpp
    src/controllers/OrderController.cpp
    src/views/MainWindow.cpp
    src/views/auth/LoginDialog.cpp
    src/views/dashboard/DashboardWidget.cpp
    src/utils/Logger.cpp
    src/utils/Validator.cpp
)

# Header files
set(HEADERS
    src/database/DatabaseManager.h
    src/models/BaseModel.h
    src/models/Admin.h
    src/models/Customer.h
    src/models/Product.h
    src/models/Order.h
    src/controllers/AuthController.h
    src/controllers/OrderController.h
    src/views/MainWindow.h
    src/views/auth/LoginDialog.h
    src/views/dashboard/DashboardWidget.h
    src/utils/Logger.h
    src/utils/Validator.h
)

# UI files
set(UI_FILES
    src/views/MainWindow.ui
    src/views/auth/LoginDialog.ui
    src/views/dashboard/DashboardWidget.ui
)

# Resource files
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::PrintSupport
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/database
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/controllers
    ${CMAKE_SOURCE_DIR}/src/views
    ${CMAKE_SOURCE_DIR}/src/utils
)

# Windows specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
```

---

## src/main.cpp

```cpp
#include <QApplication>
#include <QMessageBox>
#include "database/DatabaseManager.h"
#include "views/auth/LoginDialog.h"
#include "views/MainWindow.h"
#include "utils/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    QApplication::setApplicationName("Percetakan POS");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("YourCompany");
    
    // Initialize logger
    Logger::instance().setLogFile("percetakan_pos.log");
    Logger::instance().log(Logger::Info, "Application started");
    
    // Initialize database
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.initialize("percetakan.db")) {
        QMessageBox::critical(nullptr, "Database Error", 
            "Failed to initialize database: " + dbManager.lastError());
        return 1;
    }
    
    // Show login dialog
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        // Login successful, show main window
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    }
    
    return 0;
}
```

---

## src/database/DatabaseManager.h

```cpp
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseManager : public QObject
{
    Q_OBJECT
    
public:
    // Singleton pattern
    static DatabaseManager& instance();
    
    // Database operations
    bool initialize(const QString& dbPath);
    bool isOpen() const;
    QSqlDatabase& database();
    
    // Transaction management
    bool beginTransaction();
    bool commit();
    bool rollback();
    
    // Utility
    QString lastError() const;
    bool executeSqlFile(const QString& filePath);
    bool backup(const QString& backupPath);
    bool restore(const QString& backupPath);
    
    // Prevent copying
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    QSqlDatabase m_database;
    QString m_lastError;
    
    bool createTables();
    bool migrateDatabase();
};

#endif // DATABASEMANAGER_H
```

---

## src/database/DatabaseManager.cpp

```cpp
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::initialize(const QString& dbPath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qCritical() << "Database open failed:" << m_lastError;
        return false;
    }
    
    // Enable foreign keys
    QSqlQuery query(m_database);
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        m_lastError = query.lastError().text();
        qWarning() << "Failed to enable foreign keys:" << m_lastError;
    }
    
    qDebug() << "Database initialized successfully";
    return true;
}

bool DatabaseManager::isOpen() const
{
    return m_database.isOpen();
}

QSqlDatabase& DatabaseManager::database()
{
    return m_database;
}

bool DatabaseManager::beginTransaction()
{
    return m_database.transaction();
}

bool DatabaseManager::commit()
{
    return m_database.commit();
}

bool DatabaseManager::rollback()
{
    return m_database.rollback();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::executeSqlFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Cannot open SQL file: " + filePath;
        return false;
    }
    
    QTextStream in(&file);
    QString sqlContent = in.readAll();
    file.close();
    
    QSqlQuery query(m_database);
    
    // Split by semicolon and execute each statement
    QStringList statements = sqlContent.split(';', Qt::SkipEmptyParts);
    
    for (const QString& statement : statements) {
        QString trimmed = statement.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith("--")) {
            continue;
        }
        
        if (!query.exec(trimmed)) {
            m_lastError = query.lastError().text();
            qCritical() << "SQL execution failed:" << m_lastError;
            qCritical() << "Statement:" << trimmed;
            return false;
        }
    }
    
    return true;
}

bool DatabaseManager::backup(const QString& backupPath)
{
    // Close current connection
    QString dbName = m_database.databaseName();
    m_database.close();
    
    // Copy database file
    QFile::remove(backupPath);
    bool success = QFile::copy(dbName, backupPath);
    
    // Reopen database
    m_database.open();
    
    return success;
}

bool DatabaseManager::restore(const QString& backupPath)
{
    QString dbName = m_database.databaseName();
    m_database.close();
    
    QFile::remove(dbName);
    bool success = QFile::copy(backupPath, dbName);
    
    m_database.open();
    
    return success;
}
```

---

## src/models/BaseModel.h

```cpp
#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QObject>
#include <QDateTime>
#include <QVariantMap>

class BaseModel : public QObject
{
    Q_OBJECT
    
public:
    explicit BaseModel(QObject *parent = nullptr);
    virtual ~BaseModel();
    
    // Common fields
    int id() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    QDateTime createdAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    
    QDateTime updatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }
    
    // Validation
    virtual bool isValid() const = 0;
    virtual QString validationError() const { return m_validationError; }
    
    // Serialization
    virtual QVariantMap toMap() const = 0;
    virtual void fromMap(const QVariantMap& map) = 0;
    
protected:
    int m_id;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QString m_validationError;
};

#endif // BASEMODEL_H
```

---

## src/models/Customer.h

```cpp
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "BaseModel.h"
#include <QString>

class Customer : public BaseModel
{
    Q_OBJECT
    
public:
    explicit Customer(QObject *parent = nullptr);
    
    // Getters
    QString customerCode() const { return m_customerCode; }
    QString namaLengkap() const { return m_namaLengkap; }
    QString customerType() const { return m_customerType; }
    QString email() const { return m_email; }
    QString nomorTelp() const { return m_nomorTelp; }
    QString alamat() const { return m_alamat; }
    QString kota() const { return m_kota; }
    QString kodePos() const { return m_kodePos; }
    int priceLevelId() const { return m_priceLevelId; }
    bool isActive() const { return m_isActive; }
    int totalOrders() const { return m_totalOrders; }
    double totalSpent() const { return m_totalSpent; }
    
    // Setters
    void setCustomerCode(const QString& code) { m_customerCode = code; }
    void setNamaLengkap(const QString& nama) { m_namaLengkap = nama; }
    void setCustomerType(const QString& type) { m_customerType = type; }
    void setEmail(const QString& email) { m_email = email; }
    void setNomorTelp(const QString& telp) { m_nomorTelp = telp; }
    void setAlamat(const QString& alamat) { m_alamat = alamat; }
    void setKota(const QString& kota) { m_kota = kota; }
    void setKodePos(const QString& pos) { m_kodePos = pos; }
    void setPriceLevelId(int id) { m_priceLevelId = id; }
    void setIsActive(bool active) { m_isActive = active; }
    void setTotalOrders(int total) { m_totalOrders = total; }
    void setTotalSpent(double spent) { m_totalSpent = spent; }
    
    // BaseModel interface
    bool isValid() const override;
    QVariantMap toMap() const override;
    void fromMap(const QVariantMap& map) override;
    
private:
    QString m_customerCode;
    QString m_namaLengkap;
    QString m_customerType;
    QString m_email;
    QString m_nomorTelp;
    QString m_alamat;
    QString m_kota;
    QString m_kodePos;
    int m_priceLevelId;
    bool m_isActive;
    int m_totalOrders;
    double m_totalSpent;
};

#endif // CUSTOMER_H
```

---

## .gitignore

```gitignore
# Qt Creator
*.autosave
*.user
*.user.*

# C++ objects and libs
*.slo
*.lo
*.o
*.a
*.la
*.lai
*.so
*.dll
*.dylib

# Qt-es
object_script.*.Release
object_script.*.Debug
*_plugin_import.cpp
/.qmake.cache
/.qmake.stash
*.pro.user
*.pro.user.*
*.qbs.user
*.qbs.user.*
*.moc
moc_*.cpp
moc_*.h
qrc_*.cpp
ui_*.h
*.qmlc
*.jsc
Makefile*
*build-*

# Qt unit tests
target_wrapper.*

# QtCreator
*.qmlproject.user
*.qmlproject.user.*

# QtCreator Qml
*.qmlproject.user
*.qmlproject.user.*

# QtCreator CMake
CMakeLists.txt.user*

# Build directories
build/
build-*/
debug/
release/

# Database files
*.db
*.db-shm
*.db-wal
*.sqlite

# Logs
*.log

# Backups
*.bak
*~

# IDE
.vscode/
.idea/
*.swp
*.swo

# OS
.DS_Store
Thumbs.db
```

---

## resources/styles/default.qss

```css
/* Global Styles */
* {
    font-family: "Segoe UI", Arial, sans-serif;
    font-size: 10pt;
}

/* Main Window */
QMainWindow {
    background-color: #f5f5f5;
}

/* Buttons */
QPushButton {
    background-color: #2196F3;
    color: white;
    border: none;
    padding: 8px 16px;
    border-radius: 4px;
    min-width: 80px;
}

QPushButton:hover {
    background-color: #1976D2;
}

QPushButton:pressed {
    background-color: #0D47A1;
}

QPushButton:disabled {
    background-color: #BDBDBD;
    color: #757575;
}

/* Primary Button */
QPushButton[primary="true"] {
    background-color: #4CAF50;
}

QPushButton[primary="true"]:hover {
    background-color: #388E3C;
}

/* Danger Button */
QPushButton[danger="true"] {
    background-color: #F44336;
}

QPushButton[danger="true"]:hover {
    background-color: #D32F2F;
}

/* Line Edit */
QLineEdit {
    padding: 6px;
    border: 1px solid #BDBDBD;
    border-radius: 4px;
    background-color: white;
}

QLineEdit:focus {
    border: 2px solid #2196F3;
}

/* Table View */
QTableView {
    background-color: white;
    border: 1px solid #E0E0E0;
    gridline-color: #E0E0E0;
}

QTableView::item {
    padding: 5px;
}

QTableView::item:selected {
    background-color: #2196F3;
    color: white;
}

QHeaderView::section {
    background-color: #FAFAFA;
    padding: 8px;
    border: none;
    border-bottom: 2px solid #E0E0E0;
    font-weight: bold;
}

/* Toolbar */
QToolBar {
    background-color: white;
    border-bottom: 1px solid #E0E0E0;
    spacing: 3px;
    padding: 4px;
}

/* Status Bar */
QStatusBar {
    background-color: white;
    border-top: 1px solid #E0E0E0;
}

/* Group Box */
QGroupBox {
    border: 1px solid #E0E0E0;
    border-radius: 4px;
    margin-top: 10px;
    padding-top: 10px;
    font-weight: bold;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 5px;
}
```

---

**File ini berisi struktur lengkap project beserta contoh implementasi kode starter. Copy struktur ini dan mulai coding!**
