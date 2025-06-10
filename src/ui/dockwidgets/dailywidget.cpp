#include "dailywidget.h"
#include "../files/ui_dailywidget.h"
#include "database.h"
#include "usermanager.h"
#include "mainwindow.h"
#include <QDate>
#include <QMenu>
#include <QAction>
#include <QSqlQuery>
#include <QSqlTableModel>

#include <QtDebug>

DailyWidget::DailyWidget(Database* _d, QWidget* parent) :
ui(new Ui::DailyWidget), db(_d), QWidget(parent) {
    ui->setupUi(this);
    auto *model = db->getTableModel("invoices");
    auto userManager = db->findChild<UserManager*>("userManager");
    connect(userManager, &UserManager::userLoggedIn, this, &DailyWidget::reloadData);
    connect(model, &QSqlTableModel::modelReset, this, &DailyWidget::reloadData);
    model = db->getTableModel("orders");
    connect(model, &QSqlTableModel::modelReset, this, &DailyWidget::reloadData);
    model = db->getTableModel("payments");
    connect(model, &QSqlTableModel::modelReset, this, &DailyWidget::reloadData);
    // reloadData();
}

DailyWidget::~DailyWidget() {
    delete ui;
}

void DailyWidget::reloadData() {
    QSqlRecord user = db->findChild<UserManager*>("userManager")->currentUserRecord();
    // qDebug() << user;
    ui->lUser->setText(QString("User : <b>%1</b>").arg(user.value("display_name").toString()));
    ui->lDate->setText(locale().toString(QDate::currentDate(), "dddd, dd/MM/yyyy"));
    QSqlQuery q;
    
    q.exec(R"--(
            SELECT tDate,
                   COALESCE(orderCreated, 0) AS orderCreated,
                   COALESCE(orderCreatedValue, 0) AS orderCreatedValue,
                   COALESCE(orderPaid, 0) AS orderPaid,
                   COALESCE(orderPaidValue, 0) AS orderPaidValue,
                   COALESCE(invoicesCreated, 0) AS invoicesCreated,
                   COALESCE(invoicesCreatedValue, 0) AS invoicesCreatedValue,
                   COALESCE(invoicesPaid, 0) AS invoicesPaid,
                   COALESCE(invoicesPaidValue, 0) AS invoicesPaidValue
            FROM (
                SELECT date('now', 'localtime') AS tDate
            ) autoDate
            -- orderCreated
            LEFT JOIN
            (
                SELECT date(od.created_utc, 'localtime') AS locTime,
                       COUNT(od.order_id) AS orderCreated
                FROM orders od
                GROUP BY date(od.created_utc, 'localtime')
            ) pa ON pa.locTime = autoDate.tDate
            -- orderCreatedValue
            LEFT JOIN
            (
                SELECT date(od.created_utc, 'localtime') AS locTime,
                       SUM(oc.price) AS orderCreatedValue
                FROM orders od
                     LEFT JOIN orders_calc oc ON od.order_id = oc.order_id
                GROUP BY date(od.created_utc, 'localtime')
            ) pb ON pb.locTime = autoDate.tDate
            -- orderPaid
            LEFT JOIN
            (
                SELECT date(py.created_utc, 'localtime') AS locTime,
                       COUNT(DISTINCT od.order_id) AS orderPaid
                FROM payments py
                     LEFT JOIN invoices inv ON py.invoice_id = inv.invoice_id
                     LEFT JOIN orders od ON od.invoice_id = inv.invoice_id
                GROUP BY date(py.created_utc, 'localtime')
            ) pc ON pc.locTime = autoDate.tDate
            -- orderPaidValue
            LEFT JOIN
            (
                SELECT date(p.created_utc, 'localtime') AS locTime,
                       SUM(oc.price) AS orderPaidValue
                FROM (
                    SELECT py.created_utc,
                           od.order_id
                    FROM payments py
                         LEFT JOIN invoices inv ON py.invoice_id = inv.invoice_id
                         LEFT JOIN orders od ON od.invoice_id = inv.invoice_id
                    GROUP BY date(py.created_utc, 'localtime'), od.order_id
                ) p
                LEFT JOIN orders od ON p.order_id = od.order_id
                LEFT JOIN orders_calc oc ON oc.order_id = od.order_id
                GROUP BY date(p.created_utc, 'localtime')
            ) pd ON pd.locTime = autoDate.tDate
            -- invoicesCreated
            LEFT JOIN
            (
                SELECT date(inv.created_utc, 'localtime') AS locTime,
                       COUNT(inv.invoice_id) AS invoicesCreated
                FROM invoices inv
                GROUP BY date(inv.created_utc, 'localtime')
            ) pe ON pe.locTime = autoDate.tDate
            -- invoicesCreatedValue slotted
            LEFT JOIN
            (
                SELECT date(inv.created_utc, 'localtime') AS locTime,
                       SUM(inv.total_amount) AS invoicesCreatedValue
                FROM invoices inv
                GROUP BY date(inv.created_utc, 'localtime')
            ) pf ON pf.locTime = autoDate.tDate
            -- invoicesPaid
            LEFT JOIN
            (
                SELECT date(py.created_utc, 'localtime') AS locTime,
                       COUNT(DISTINCT inv.invoice_id) AS invoicesPaid
                FROM payments py
                     LEFT JOIN invoices inv ON py.invoice_id = inv.invoice_id
                GROUP BY date(py.created_utc, 'localtime')
            ) pg ON pg.locTime = autoDate.tDate
            -- invoicesPaidValue
            LEFT JOIN
            (
                SELECT date(py_inv.created_utc, 'localtime') AS locTime,
                       SUM(py_inv.total_amount) AS invoicesPaidValue
                FROM (
                    SELECT py.created_utc,
                           inv.invoice_id,
                           inv.total_amount
                    FROM payments py
                         LEFT JOIN invoices inv ON py.invoice_id = inv.invoice_id
                    GROUP BY date(py.created_utc, 'localtime'), inv.invoice_id
                ) py_inv
                GROUP BY date(py_inv.created_utc, 'localtime')
            ) ph ON ph.locTime = autoDate.tDate; )--");
    q.next();
    auto rec = q.record();
    ui->label_ordersSavedCount->setText(locale().toString(rec.value("orderCreated").toInt()));
    ui->label_ordersSavedValueSum->setText(locale().toString(rec.value("orderCreatedValue").toInt()));
    ui->label_ordersPaidCount->setText(locale().toString(q.value("orderPaid").toInt()));
    ui->label_ordersPaidValueSum->setText(locale().toString(q.value("orderPaidValue").toInt()));
    ui->label_invoiceSavedCount->setText(locale().toString(q.value("invoicesCreated").toInt()));
    ui->label_invoiceSavedValueSum->setText(locale().toString(q.value("invoicesCreatedValue").toInt()));
    ui->label_invoicePaidCount->setText(locale().toString(q.value("invoicesPaid").toInt()));
    ui->label_invoicePaidValueSum->setText(locale().toString(q.value("invoicesPaidValue").toInt()));
}

DailyDockWidget::DailyDockWidget(Database* _d, MainWindow* p) :
    QDockWidget(p) {
    DailyWidget* dw = new DailyWidget(_d, this);
    setWidget(dw);
    QMenu* menuView = findChild<QMenu*>("menuView");
    if(menuView) {
        menuView->insertAction(nullptr, this->toggleViewAction());
    }
    setWindowTitle("Ringkasan Harian");
}

DailyDockWidget::~DailyDockWidget(){}