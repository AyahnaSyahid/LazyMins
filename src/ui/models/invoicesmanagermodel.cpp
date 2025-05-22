#include "invoicesmanagermodel.h"
#include "database.h"

#include <QModelIndex>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QtDebug>

const QString myQuery(R"--(
WITH OrdersValue AS (
    SELECT order_id,
           invoice_id,
           discount,
           CASE WHEN use_area = 1 THEN CEIL(width * height * unit_price * quantity / 500) * 500 ELSE unit_price * quantity END AS sub_total_price,
           CASE WHEN use_area = 1 THEN CEIL(width * height * production_cost * quantity / 500) * 500 ELSE production_cost * quantity END AS sub_total_cost
      FROM orders
),
OrdersSums AS (
    SELECT invoice_id,
           COUNT(order_id) AS order_count,
           SUM(discount) AS discount,
           SUM(sub_total_price) AS total_price,
           SUM(sub_total_cost) AS total_cost
      FROM OrdersValue
     GROUP BY invoice_id
)
SELECT invoices.invoice_id,
       invoices_dcode.invoice_code,
       customers.name,
       order_count,
       total_price,
       total_cost,
       OrdersSums.discount AS order_discount,
       invoices.discount AS invoice_discount,
       total_price - total_cost - invoices.discount AS netto,
       ROUND((total_price - total_cost) * 100.0 / total_price, 3) AS net_percent,
       ROUND((total_price - total_cost - OrdersSums.discount) * 100.0 / total_price, 3) AS netwodisc_percent,
       ROUND((total_price - total_cost - OrdersSums.discount - invoices.discount) * 100.0 / total_price, 3) AS netwoidisc_percent
  FROM invoices
       JOIN
       invoices_dcode USING (
           invoice_id
       )
       JOIN
       customers ON customers.customer_id = invoices.customer_id
       JOIN
       OrdersSums USING (
           invoice_id
       );
)--");

InvoicesManagerModel::InvoicesManagerModel(Database* _d, QObject* parent) :
db(_d), QSortFilterProxyModel(parent) {
    QSqlQueryModel* qModel = new QSqlQueryModel(this);
    qModel->setObjectName("qModel");
    qModel->setQuery(myQuery);
    setSourceModel(qModel);
    qDebug() << qModel->lastError();
}

InvoicesManagerModel::~InvoicesManagerModel() {}

QVariant InvoicesManagerModel::data(const QModelIndex& mi, int role) const {
    return QSortFilterProxyModel::data(mi, role);
}