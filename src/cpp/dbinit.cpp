#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QtDebug>

#include "databasequeries.h"

namespace DBInitNS {

typedef const char* CC_TABLE_QUERY;

CC_TABLE_QUERY ProductCreateTable = ""
      "CREATE TABLE IF NOT EXISTS product (id INTEGER PRIMARY KEY, "
              "code TEXT  NOT NULL UNIQUE COLLATE NOCASE DEFAULT 'KodeBarang', "
              "name TEXT NOT NULL, "
              "qr TEXT NOT NULL, "
              "pcost REAL NOT NULL, "
              "psell REAL NOT NULL, "
              "area INTEGER DEFAULT 0, "
              "desc TEXT NOT NULL, "
              "ctime TEXT, mtime TEXT)";

CC_TABLE_QUERY CustomerCreateTable = ""
"CREATE TABLE IF NOT EXISTS customer ( id INTEGER PRIMARY KEY, "
              "name TEXT NOT NULL UNIQUE COLLATE NOCASE, "
              "address TEXT NOT NULL DEFAULT '', "
              "phone1 TEXT NOT NULL DEFAULT '', "
              "phone2 TEXT NOT NULL DEFAULT '', "
              "email TEXT NOT NULL DEFAULT '', "
              "ctime TEXT,"
              "mtime TEXT)";

CC_TABLE_QUERY PaymentCreateTable = ""
"CREATE TABLE IF NOT EXISTS payment ( id INTEGER PRIMARY KEY, "
              "disc REAL NOT NULL, "    // total diskon
              "ctime TEXT, "
              "mtime TEXT)";

CC_TABLE_QUERY RepaymentCreateTable = ""
"CREATE TABLE IF NOT EXISTS repayment (id INTEGER PRIMARY KEY, "
    "payment_id REFERENCES payment(id), "
    "cash REAL NOT NULL DEFAULT 0, "
    "trf REAL NOT NULL DEFAULT 0, "
    "ctime TEXT, "
    "mtime TEXT )";

CC_TABLE_QUERY SaleCreateTable = ""
"CREATE TABLE IF NOT EXISTS sale ( id INTEGER PRIMARY KEY, "
              "customer_id REFERENCES customer(id), "
              "product_id REFERENCES product(id) NOT NULL, "
              "name TEXT NOT NULL DEFAULT 'Untitled1', "
              "sizeW REAL NOT NULL DEFAULT 1.0, "
              "sizeH REAL NOT NULL DEFAULT 1.0, "
              "quantity REAL NOT NULL DEFAULT 1.0, "
              "netprice REAL NOT NULL DEFAULT 1.0, "
              "sellprice REAL NOT NULL DEFAULT 1.0, "
              "payment_id REFERENCES payment(id), "
              "ptime TEXT NOT NULL DEFAULT '', "
              "ctime TEXT NOT NULL DEFAULT '', "
              "mtime TEXT NOT NULL DEFAULT '')";
              
CC_TABLE_QUERY StoreGoodsCreateTable = ""
"CREATE TABLE IF NOT EXISTS store_goods ( id INTEGER PRIMARY KEY, "
              "name TEXT UNIQUE COLLATE NOCASE, "
              "desc TEXT NOT NULL DEFAULT '', "
              "ctime TEXT, mtime TEXT )";

CC_TABLE_QUERY StoreLogsCreateTable = ""
"CREATE TABLE IF NOT EXISTS store_logs ( gd_id REFERENCES store_goods(id) NOT NULL, "
              "amount REAL NOT NULL, "
              "store_type INTEGER NOT NULL DEFAULT 1, "
              "ctime TEXT, mtime TEXT )";

CC_TABLE_QUERY UserCreateTable = ""
"CREATE TABLE user (id INTEGER PRIMARY KEY, "
             "username TEXT UNIQUE COLLATE NOCASE, "
             "password BLOB NOT NULL, "
             "display_name TEXT NOT NULL DEFAULT 'Admin', "
             "permission INTEGER NOT NULL DEFAULT 1, "
             "ctime TEXT, "
             "last_log_time TEXT)";


bool executeQuery(const QString& query, QSqlQuery& qq) {
    qq.prepare(query);
    if (!qq.exec()) {
        qDebug() << "Error : " << qq.lastError().text();
        return false;
    }
    return true;
}

} // end DBInitNS

namespace DB_INIT
{
  bool initializeDatabase() {
    using namespace DBInitNS;
    QSqlQuery q;
    // return q.exec("PRAGMA foreign_keys = OFF") &
           // executeQuery(UserCreateTable, q) &
           // executeQuery(ProductCreateTable, q) &
           // executeQuery(CustomerCreateTable, q) &
           // executeQuery(PaymentCreateTable, q) &
           // executeQuery(RepaymentCreateTable, q) &
           // executeQuery(SaleCreateTable, q) &
           // executeQuery(StoreGoodsCreateTable, q) &
           // executeQuery(StoreLogsCreateTable, q) &
           // q.exec("PRAGMA foreign_keys = ON");
  
    return q.exec("PRAGMA foreign_keys = OFF") &&
           executeQuery(DatabaseQueries::createUsersTable(), q) &&
           executeQuery(DatabaseQueries::createRolesTable(), q) &&
           executeQuery(DatabaseQueries::createUserRolesTable(), q) &&
           executeQuery(DatabaseQueries::createPermissionsTable(), q) &&
           executeQuery(DatabaseQueries::createRolePermissionsTable(), q) &&
           executeQuery(DatabaseQueries::createCustomersTable(), q) &&
           executeQuery(DatabaseQueries::createProductsTable(), q) &&
           executeQuery(DatabaseQueries::createOrdersTable(), q) &&
           executeQuery(DatabaseQueries::createInvoicesTable(), q) &&
           executeQuery(DatabaseQueries::createInvoicePaymentsTable(), q) &&
           executeQuery(DatabaseQueries::createDivisionsTable(), q) &&
           executeQuery(DatabaseQueries::createExpensesTable(), q) &&
           q.exec("PRAGMA foreign_keys = ON");
  }
  
}
