#include "customerorderswidget.h"
#include "../files/ui_customerorderswidget.h"
#include "viewer/orderviewdialog.h"
#include "viewer/invoiceviewdialog.h"
#include "database.h"
#include "mainwindow.h"
#include "usermanager.h"
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
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
: ui(new Ui::CustomerOrdersWidget), db(_d), model(new QSqlQueryModel(this)), QWidget(parent)
{
    ui->setupUi(this);
    connect(_d->getTableModel("orders"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("invoices"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("customers"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    connect(_d->getTableModel("payments"), &QSqlTableModel::modelReset, this, &CustomerOrdersWidget::reloadData);
    model->setQuery(R"--(
          SELECT c.customer_id,
                 c.name AS Konsumen,
                 COUNT(DISTINCT CASE WHEN o.invoice_id IS NULL AND 
                                          o.status = 'OK' THEN o.order_id END) AS Orders,
                 COUNT(DISTINCT CASE WHEN i.paid = 0 THEN i.invoice_id END) AS Inv,
                 COUNT(DISTINCT CASE WHEN i.payment_count > 0 AND 
                                          i.unpaid > 0 THEN i.invoice_id END) AS [Inv (P)]
            FROM customers c
                 LEFT JOIN
                 orders o ON c.customer_id = o.customer_id
                 LEFT JOIN
                 invoices_summary i ON c.name = i.name-- atau gunakan c.customer_id = i.customer_id jika ada
           GROUP BY c.name
          HAVING Orders > 0 OR 
                 Inv > 0 OR 
                 [Inv (P)] > 0
           ORDER BY Orders DESC,
                    INV DESC,
                    [INV (P)],
                    Konsumen;
    )--");
	auto proxy = new _Proxy(this);
	proxy->setSourceModel(model);
	proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxy->setFilterKeyColumn(1);
	connect(ui->lineEdit, &QLineEdit::textChanged, proxy, &_Proxy::setFilterFixedString); 
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
  auto clickedIndex = model->index(ui->customerOrdersTable->rowAt(_p.y()), 0);
  
  if(!clickedIndex.isValid()) return;
  auto sCustomerId = clickedIndex.data(Qt::EditRole);
  QMenu contextMenu("Atur", this);
  if(clickedIndex.siblingAtColumn(2).data(Qt::EditRole).toBool()) {
    auto aOrder = contextMenu.addAction("Lihat Order");
    connect(aOrder, &QAction::triggered, [this, &sCustomerId]() { showOrdersFor(sCustomerId); });
  }
  if(clickedIndex.siblingAtColumn(3).data(Qt::EditRole).toBool() || clickedIndex.siblingAtColumn(4).data(Qt::EditRole).toBool()) {
    auto aInvoice = contextMenu.addAction("Lihat Invoice");
    connect(aInvoice, &QAction::triggered,[this, &sCustomerId]() { showInvoicesFor(sCustomerId); });
  }  
	if(contextMenu.actions().size()) 
    contextMenu.exec(showPoint);
}

void CustomerOrdersWidget::showOrdersFor(const QVariant& cn) {
	OrderViewDialog* d = new OrderViewDialog(cn.toInt(), this);
  d->setAttribute(Qt::WA_DeleteOnClose);
  connect(model, &QSqlQueryModel::modelReset, d, &OrderViewDialog::reloadData);
  connect(d, &OrderViewDialog::createInvoiceForOrders, this, &CustomerOrdersWidget::createInvoiceForOrders);
  connect(d, &OrderViewDialog::editOrder, this, &CustomerOrdersWidget::editOrder);
  d->open();
}

void CustomerOrdersWidget::showInvoicesFor(const QVariant& cn) {
	InvoiceViewDialog* vd = new InvoiceViewDialog(cn.toInt(), db, this);
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
    
    QMenu* menuView = parent->menuBar()->findChild<QMenu*>("menuView");
    if(menuView) {
        menuView->insertAction(nullptr, toggleViewAction());
    }
    setWindowTitle("Order Konsumen");
    auto uman = _d->findChild<UserManager*>("userManager");
    connect(uman, &UserManager::userLoggedIn, this, &CustomerOrdersDockWidget::currentUserChanged);
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

void CustomerOrdersDockWidget::currentUserChanged(int uid) {
  if(UserManager::hasPermission(uid, "GRANT_EVERYTHING")) {
    return show();
  }
  if(! ( UserManager::hasPermission(uid, "ManageOrders") && UserManager::hasPermission(uid, "ManageInvoices") ) ) {
    hide();
  }
}