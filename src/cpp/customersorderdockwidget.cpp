#include "customersorderdockwidget.h"
#include "ui_customersorderdockwidget.h"
#include "editorderdialog.h"
#include "userpermissions.h"
#include "notifier.h"
#include "orderitem.h"
#include "helper.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QAction>
#include <QPoint>
#include <QMenu>
#include <QColor>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTime>
#include <QtMath>
#include <QtDebug>

#include <QSortFilterProxyModel>

class CustomersOrderProxy : public QSortFilterProxyModel {
public:
    CustomersOrderProxy(QObject* parent) : QSortFilterProxyModel(parent) {}
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const override {
        auto columnName = headerData(mi.column(), Qt::Horizontal).toString();
        auto def = QSortFilterProxyModel::data(mi, role);
        QLocale loc;
        if(columnName == "Nilai" || columnName == "Sisa") {
            if(role == Qt::TextAlignmentRole) {
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            }
            if(role == Qt::DisplayRole) {
                return loc.toString(def.toLongLong());
            }
        }
        if(columnName == "Tanggal") {
            if(role == Qt::DisplayRole) {
                return loc.toString(def.toDateTime().toLocalTime(), "dddd, d MMM yyyy");
            }
            if(role == Qt::TextAlignmentRole) {
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            }
        }
        if(role == Qt::BackgroundRole) {
            if(mi.siblingAtColumn(8).data(Qt::EditRole).toLongLong() < 1) {
                return QColor(0xffccffdd);
            }
        }
        return def;
    }
};

CustomersOrderDockWidget::CustomersOrderDockWidget(QWidget* parent, Qt::WindowFlags flags)
: ui(new Ui::CustomersOrderDockWidget), QDockWidget(parent, flags) {
    ui->setupUi(this);
    auto queryModel = new QSqlQueryModel(this);
    queryModel->setObjectName("queryModel");
    queryModel->setQuery(baseQuery());
    auto proxy = new CustomersOrderProxy(this);
    proxy->setObjectName("customersOrderProxy");
    proxy->setSourceModel(queryModel);
    ui->tableView->setModel(proxy);
    auto hh = ui->tableView->horizontalHeader();
    hh->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(hh, &QHeaderView::customContextMenuRequested, this, &CustomersOrderDockWidget::headerCustomContextMenu);
    connect(ui->checkBox, &QCheckBox::toggled, this, &CustomersOrderDockWidget::refreshModel); 
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &CustomersOrderDockWidget::refreshModel); 
    
    
    ui->actionEditOrder->setProperty("enableWithPerm", "ManageOrder");
    ui->actionHapusOrder->setProperty("enableWithPerm", "ManageOrder");
    ui->tableView->addAction(ui->actionEditOrder);
    ui->tableView->addAction(ui->actionHapusOrder);
    // ui->tableView->addAction(ui->actionView);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(5);
    ui->tableView->resizeColumnsToContents();
    
    refreshModel();
    
    connect(dbNot, &DatabaseNotifier::tableChanged, this, &CustomersOrderDockWidget::onTableChanged);
}

CustomersOrderDockWidget::~CustomersOrderDockWidget() {
    delete ui;
}

QString CustomersOrderDockWidget::baseQuery() const {
    return R"-(
WITH order_price AS(SELECT order_id,
                           total_price
                      FROM orders)
SELECT cus.customer_id AS CID,
       ord.order_id AS OID,
       ord.order_date AS Tanggal,
       cus.customer_name AS Konsumen,
       ord.order_name AS [Order],
       ord.product_id AS PID,
       pro.code AS Product,
       ord.total_price AS Nilai,
       CASE WHEN COALESCE(inv.total_paid, 0) = 0 THEN 
            COALESCE(inv.total_amount, ord.total_price) ELSE 
                inv.total_amount - inv.total_paid - inv.discount_amount END AS Sisa
  FROM orders ord
       JOIN
       order_price op USING(order_id)
       JOIN
       customers cus USING(customer_id)
       JOIN
       products pro USING(product_id)
       LEFT JOIN
       invoices inv USING(invoice_id)
    )-";
}

QString CustomersOrderDockWidget::limitQuery() const {
    return QString(" LIMIT %1 OFFSET %2 ").arg(_limit).arg((_currentPage - 1) * _limit);
}

QString CustomersOrderDockWidget::whereQuery() const {
    QStringList where;
    auto kof = ui->lineEdit->text().trimmed();
    if(!kof.isEmpty()) {
        where << QString(" Konsumen || ' ' || [Order] LIKE '%%1%'").arg(kof);
    }
    
    if(!ui->checkBox->isChecked()) {
        where << QString("Sisa > 0");
    }
    
    return where.join(" AND ");
}

void CustomersOrderDockWidget::refreshModel() {
    QSqlQuery q(QString("SELECT COUNT(CID) FROM (%1)").arg(baseQuery()));
    
    if(q.lastError().isValid()) {
        qDebug() << q.lastError().text();
    }
    
    q.next();
    
    _totalPage = qCeil(q.value(0).toDouble() / _limit);
    _currentPage = qBound(1, _currentPage, _totalPage);
    
    ui->spinBox->blockSignals(true);
    ui->spinBox->setValue(_currentPage);
    ui->spinBox->setSuffix(QString(" / %1").arg(_totalPage));
    ui->spinBox->blockSignals(false);
    
    auto qm = findChild<QSqlQueryModel*>("queryModel");
    if(qm) {
        auto where = whereQuery();
        if(where.isEmpty()) {
            qm->setQuery(baseQuery() + limitQuery());
        } else {
            auto qq = QString(" %1 WHERE %2 %3 ").arg(baseQuery(), whereQuery(), limitQuery());
            // qDebug() << qq;
            qm->setQuery(qq);
        }
        if(qm->lastError().isValid()) {
            qDebug() << qm->lastError().text();
        }
    }
    
    ui->firstButton->setDisabled(_currentPage == 1);
    ui->prevButton->setDisabled(_currentPage != 1);
    ui->nextButton->setDisabled(_currentPage < _totalPage);
    ui->lastButton->setDisabled(_currentPage == _totalPage);
    ui->navPaging->setHidden(_totalPage < 2);
}

void CustomersOrderDockWidget::headerCustomContextMenu(const QPoint& pos) {
    auto hh = ui->tableView->horizontalHeader();
    auto npos = hh->viewport()->mapToGlobal(pos);
    auto model = ui->tableView->model();
    QMenu menu;
    QList<int> needAction {2, 3, 4, 6, 7, 8};
    
    for(int i=0; i<needAction.count(); ++i) {
        auto cn = needAction.at(i);
        auto act = menu.addAction(model->headerData(cn, Qt::Horizontal).toString());
        act->setCheckable(true);
        act->setChecked(!hh->isSectionHidden(cn));
        act->setProperty("underlyingColumn", cn);
    }
    auto act = menu.exec(npos);
    if(act) {
        hh->setSectionHidden(act->property("underlyingColumn").toInt(), !act->isChecked());
    }
}

void CustomersOrderDockWidget::on_actionEditOrder_triggered() {
    if(!UserPermissions::hasPermission(PermissionItem("ModifyOrder"))) {
        MessageHelper::warning(this, "Peringatan", "Anda tidak memiliki hak untuk mengubah order");
        return;
    }
    if(ui->tableView->selectionModel()->hasSelection()) {
        auto ix = ui->tableView->selectionModel()->selectedIndexes()[0]
                    .siblingAtColumn(1).data().toLongLong();
        // qDebug() << "Selected ix " << ix;
        auto eod = new EditOrderDialog(this);
        eod->setAttribute(Qt::WA_DeleteOnClose);
        connect(eod, &QDialog::accepted, [](){ dbNot->emitChanged("orders");});
        eod->setOrder(ix);
        eod->open();
    }
}

void CustomersOrderDockWidget::on_tableView_doubleClicked(const QModelIndex& mi) {
    if(!UserPermissions::hasPermission(PermissionItem("ModifyOrder"))) {
        MessageHelper::warning(this, "Peringatan", "Anda tidak memiliki hak untuk mengubah order");
        return;
    }
    ui->actionEditOrder->trigger();
}

void CustomersOrderDockWidget::on_actionHapusOrder_triggered() {
    if(!UserPermissions::hasPermission(PermissionItem("ModifyOrder"))) {
        MessageHelper::warning(this, "Peringatan", "Anda tidak memiliki hak untuk mengubah order");
        return;
    }
    if(ui->tableView->selectionModel()->hasSelection()) {
        auto ix = ui->tableView->selectionModel()->selectedIndexes()[0]
                    .siblingAtColumn(1).data().toLongLong();
        // qDebug() << "Selected ix " << ix;
        OrderItem oi(ix);
        auto yes = MessageHelper::question(this, "Konfirmasi", "Lanjutkan menghapus?");
        if(yes == QMessageBox::Yes) {
            if(oi.erase()) {
                dbNot->emitChanged("orders");
            } else {
                MessageHelper::information(this, "Kesalahan", "Gagal menghapus order");
            }
        }
    }
}

void CustomersOrderDockWidget::on_actionView_triggered() {
    
}

void CustomersOrderDockWidget::onTableChanged(const QString &tn) {
    QStringList watched {"orders", "customers", "invoices", "invoice_payments"};
    if(watched.contains(tn)) {
        refreshModel();
    }
}
