#include "customerorderswidget.h"
#include "../files/ui_customerorderswidget.h"
#include "database.h"
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlQuery>

CustomerOrdersWidget::CustomerOrdersWidget(Database* _d, QWidget* parent)
    : ui(new Ui::CustomerOrdersWidget), model(nullptr), QWidget(parent)
{
    ui->setupUi(this);
    connect(_d->getTableModel("orders"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("invoices"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("customers"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    model = new QSqlQueryModel(this);
    ui->customerOrdersTable->setModel(model);
    model->setQuery(R"--(
    SELECT one.customer_name,
       COALESCE(never_paid, 0) AS never_paid,
       COALESCE(partial, 0) AS partial
       FROM ( SELECT name as customer_name FROM customers ) one
       LEFT JOIN ( SELECT customer_name, COALESCE(COUNT(invoice_id), 0) as never_paid FROM invoices_summary GROUP BY customer_name HAVING paid = 0 ) two ON one.customer_name = two.customer_name
       LEFT JOIN ( SELECT customer_name, COALESCE(COUNT(invoice_id), 0) AS partial FROM invoices_summary GROUP BY customer_name HAVING payment_count > 0 AND unpaid > 0) three ON one.customer_name = three.customer_name
       WHERE never_paid > 0 OR partial > 0
    )--");
}

CustomerOrdersWidget::~CustomerOrdersWidget() {
    delete ui;
}

void CustomerOrdersWidget::reloadData() {
    auto q = model->query();
    q.exec();
    model->setQuery(q);
}

CustomerOrdersDockWidget::CustomerOrdersDockWidget(Database* _d, QWidget* parent)
: QDockWidget(parent) {
    auto cu = new CustomerOrdersWidget(_d, this);
    setWidget(cu);
    setWindowTitle("Tabel Order Konsumen");
}

CustomerOrdersDockWidget::~CustomerOrdersDockWidget(){}