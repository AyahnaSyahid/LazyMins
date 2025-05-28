#include "dashboardwidget.h"
#include "files/ui_dashboardwidget.h"
#include "database.h"
#include "usermanager.h"
#include <QDate>
#include <QSqlQuery>
#include <QtDebug>
#include <QSqlTableModel>

DashboardWidget::DashboardWidget(Database* _d, QWidget* parent) :
ui(new Ui::DashboardWidget), db(_d), QWidget(parent) {
    ui->setupUi(this);
    auto *model = db->getTableModel("invoices");
    auto userManager = db->findChild<UserManager*>("userManager");
    connect(userManager, &UserManager::userLoggedIn, this, &DashboardWidget::reloadData);
    connect(model, &QSqlTableModel::modelReset, this, &DashboardWidget::reloadData);
    model = db->getTableModel("orders");
    connect(model, &QSqlTableModel::modelReset, this, &DashboardWidget::reloadData);
    model = db->getTableModel("payments");
    connect(model, &QSqlTableModel::modelReset, this, &DashboardWidget::reloadData);
    // reloadData();
}

DashboardWidget::~DashboardWidget() {
    delete ui;
}

void DashboardWidget::reloadData() {
    QSqlRecord user = db->findChild<UserManager*>("userManager")->currentUserRecord();
    // qDebug() << user;
    ui->lUser->setText(QString("User : <b>%1</b>").arg(user.value("display_name").toString()));
    ui->lDate->setText(locale().toString(QDate::currentDate(), "dddd, dd/MM/yyyy"));
    QSqlQuery q;
    
    q.exec(R"--(
            SELECT COALESCE(COUNT(ov.order_id), 0) AS JOD,
                   COALESCE(SUM(ov.price - ov.discount), 0) AS TPR,
                   today.DT AS DT
            FROM (
                SELECT date('now', 'localtime') AS DT
            ) AS today
            LEFT JOIN orders oo ON date(oo.created_utc, 'localtime') = today.DT
            LEFT JOIN orders_calc ov USING (order_id)
            GROUP BY today.DT;)--"
    );
    q.next();
    auto rec = q.record();
    ui->label_ordersSavedCount->setText(locale().toString(rec.value("JOD").toInt()));
    ui->label_ordersSavedValueSum->setText(locale().toString(rec.value("TPR").toInt()));
    
    q.exec(R"--(
            SELECT COALESCE(COUNT(ot.order_id), 0) AS OC,
                   COALESCE(SUM(py.amount), 0) AS PS
            FROM (
                SELECT date('now', 'localtime') AS payment_date
            ) AS today
            LEFT JOIN payments py ON py.payment_date = today.payment_date
            LEFT JOIN invoices inv ON py.invoice_id = inv.invoice_id
            LEFT JOIN orders ot ON ot.invoice_id = inv.invoice_id
            GROUP BY today.payment_date;
    )--");
    q.next();
    // if(q.next()) qDebug() << "Order Saved OK" ;
    
    ui->label_ordersPaidCount->setText(locale().toString(q.value("OC").toInt()));
    ui->label_ordersPaidValueSum->setText(locale().toString(q.value("PS").toInt()));
    
    q.exec(R"--(
            SELECT COUNT(DISTINCT inv.invoice_id) AS INVC,
                   COALESCE(SUM(py.amount), 0) AS INVP
              FROM ( SELECT date('now', 'localtime') AS dt ) CD
              LEFT JOIN payments py ON py.payment_date = CD.dt
              LEFT JOIN invoices inv ON py.invoice_id = inv.invoice_id
             GROUP BY CD.dt;
    )--");
    q.next();
    ui->label_invoicePaidValueSum->setText(locale().toString(q.value("INVP").toInt()));
    
    
    q.exec(R"--(
            SELECT COALESCE(COUNT(DISTINCT inv.invoice_id), 0) AS INVC
                FROM ( SELECT date('now', 'localtime') AS dt ) CD
                LEFT JOIN invoices inv ON date(inv.created_utc, 'localtime') = CD.dt;
    )--");
    q.next();
    
    ui->label_invoiceSavedCount->setText(locale().toString(q.value("INVC").toInt()));
    q.exec(R"--(
            SELECT COALESCE(COUNT(DISTINCT inv.invoice_id), 0) AS IPC
            FROM (
                SELECT date('now', 'localtime') AS DT
            ) AS today
            LEFT JOIN payments py ON date(py.payment_date, 'localtime') = today.DT
            LEFT JOIN invoices inv ON py.invoice_id = inv.invoice_id
            GROUP BY today.DT;
    )--");
    q.next();
    // if(q.next()) qDebug() << "Invoice PAID Count OK" ;
    
    ui->label_invoicePaidCount->setText(locale().toString(q.value("IPC").toInt()));
    
    q.exec(R"--(
            SELECT COALESCE(sum(oc.price - oc.discount), 0) AS IP
              FROM (
                       SELECT date('now', 'localtime') AS DT
                   )
                   CT
                   LEFT JOIN
                   invoices inv ON date(inv.created_utc, 'localtime') = CT.DT
                   LEFT JOIN
                   orders ord ON ord.invoice_id = inv.invoice_id
                   LEFT JOIN
                   orders_calc oc ON ord.order_id = oc.order_id
             GROUP BY CT.DT;
    )--");
    q.next();
    // if(q.next()) qDebug() << "Invoice SAVED SUM OK" ;
    ui->label_invoiceSavedValueSum->setText(locale().toString(q.value("IP").toInt()));
    
}