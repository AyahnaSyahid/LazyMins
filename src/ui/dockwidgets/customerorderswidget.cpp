#include "customerorderswidget.h"
#include "../files/ui_customerorderswidget.h"
#include "database.h"
#include "mainwindow.h"
#include "viewer/orderviewdialog.h"
#include "viewer/invoiceviewdialog.h"
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QtDebug>

class _Proxy : public QSortFilterProxyModel {
public:
	_Proxy(QObject* parent) : QSortFilterProxyModel(parent) {}
	QVariant data(const QModelIndex& mi, int role = Qt::DisplayRole) const override;
private:
	QLocale locale;
};

CustomerOrdersWidget::CustomerOrdersWidget(Database* _d, QWidget* parent)
    : ui(new Ui::CustomerOrdersWidget), model(nullptr), QWidget(parent)
{
    ui->setupUi(this);
    connect(_d->getTableModel("orders"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("invoices"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("customers"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("payments"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    model = new QSqlQueryModel(this);
    model->setQuery(R"--(
        SELECT c.customer_id,
               c.name AS Konsumen,
               COUNT(DISTINCT CASE WHEN o.invoice_id IS NULL AND o.status = 'OK' THEN o.order_id END) AS Orders,
               COUNT(DISTINCT CASE WHEN i.paid = 0 THEN i.invoice_id END) AS Inv,
               COUNT(DISTINCT CASE WHEN i.payment_count > 0 AND
                                        i.unpaid > 0 THEN i.invoice_id END) AS [Inv (P)]
          FROM customers c
               LEFT JOIN
               orders o ON c.customer_id = o.customer_id
               LEFT JOIN
               invoices_summary i ON c.name = i.customer_name -- atau gunakan c.customer_id = i.customer_id jika ada
         GROUP BY c.name
        HAVING [Orders] > 0 OR Inv > 0 OR [Inv (P)] > 0
        ORDER BY Orders DESC,
                  INV DESC,
                  [INV (P)],
                  Konsumen;
    )--");
	auto proxy = new _Proxy(this);
	proxy->setSourceModel(model);
	proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
	connect(ui->lineEdit, &QLineEdit::textEdited, [this, proxy](QString text) { proxy->setFilterFixedString(text);}); 
	model->setHeaderData(2, Qt::Horizontal, "Jumlah Order\nyang belum tercatat dalam invoice", Qt::ToolTipRole);
	model->setHeaderData(3, Qt::Horizontal, "Jumlah Invoice\nyang belum dibayar", Qt::ToolTipRole);
	model->setHeaderData(4, Qt::Horizontal, "Jumlah Invoice\nyang belum lunas", Qt::ToolTipRole);
    ui->customerOrdersTable->setModel(proxy);
    ui->customerOrdersTable->hideColumn(0);
    ui->customerOrdersTable->verticalHeader()->hide();
	ui->customerOrdersTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->customerOrdersTable->resizeColumnsToContents();
}

CustomerOrdersWidget::~CustomerOrdersWidget() {
    delete ui;
}

void CustomerOrdersWidget::reloadData() {
    auto q = model->query();
    q.exec();
    model->setQuery(q);
}

void CustomerOrdersWidget::on_customerOrdersTable_customContextMenuRequested(const QPoint& _p) {
	auto showPoint = ui->customerOrdersTable->viewport()->mapToGlobal(_p);
	auto sCustomerId = model->index(ui->customerOrdersTable->rowAt(_p.y()), 0).data();
	QMenu contextMenu("Atur", this);
	auto aOrder = contextMenu.addAction("Lihat Order");
	auto aInvoice = contextMenu.addAction("Lihat Invoice");
	connect(aOrder, &QAction::triggered, [this, &sCustomerId]() { showOrdersFor(sCustomerId); });
	connect(aInvoice, &QAction::triggered,[this, &sCustomerId]() { showInvoicesFor(sCustomerId); });
	contextMenu.exec(showPoint);
}

void CustomerOrdersWidget::showOrdersFor(const QVariant& cn) {
	OrderViewDialog* d = new OrderViewDialog(cn.toInt(), this);
    d->setAttribute(Qt::WA_DeleteOnClose);
    connect(d, &OrderViewDialog::createInvoiceForOrders, this, &CustomerOrdersWidget::createInvoiceForOrders);
    connect(model, &QSqlQueryModel::modelReset, d, &OrderViewDialog::reloadData);
    connect(d, &OrderViewDialog::editOrder, this, &CustomerOrdersWidget::editOrder);
    d->open();
}

void CustomerOrdersWidget::showInvoicesFor(const QVariant& cn) {
	InvoiceViewDialog* vd = new InvoiceViewDialog(cn.toInt(), this);
    vd->setAttribute(Qt::WA_DeleteOnClose);
    vd->open();
}

CustomerOrdersDockWidget::CustomerOrdersDockWidget(Database* _d, MainWindow* parent)
: QDockWidget(parent) {
    auto cu = new CustomerOrdersWidget(_d, this);
    cu->setObjectName("customerOrdersWidget");
    setWidget(cu);
    connect(cu, SIGNAL(editOrder(int)), parent, SLOT(openOrderEditor(int)));
    connect(cu, SIGNAL(createInvoiceForOrders(QList<int>)), parent, SLOT(createInvoiceForOrdersReceiver(QList<int>)));
    connect(parent, SIGNAL(createInvoiceForOrdersReceived()), cu, SIGNAL(createInvoiceForOrdersSent()));
    
    QMenu* menuView = findChild<QMenu*>("menuView");
    if(menuView) {
        menuView->insertAction(nullptr, toggleViewAction());
    }
    
    setWindowTitle("Order Konsumen");
}

CustomerOrdersDockWidget::~CustomerOrdersDockWidget(){}


QVariant _Proxy::data(const QModelIndex& mi, int role) const {
	if(role == Qt::DisplayRole) {
		if(mi.column() > 1) {
			return locale.toString(mi.data(Qt::EditRole).toInt());
		}
	} else if(role == Qt::TextAlignmentRole) {
		if(mi.column() > 1) {
			return int(Qt::AlignRight | Qt::AlignVCenter);
		}
	}
	return QSortFilterProxyModel::data(mi, role);
}