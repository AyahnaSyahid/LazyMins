#include "orderviewdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QDate>
#include <QAction>
#include <QSqlQueryModel>
#include <QWidget>
#include <QLocale>
#include <QSqlQuery>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>
#include <QtDebug>

namespace Viewer {
    class _Proxy : public QSortFilterProxyModel {
    public:
        _Proxy(QObject* parent) : loc(), QSortFilterProxyModel(parent) {}
        ~_Proxy() = default;
        QVariant data(const QModelIndex&, int =Qt::DisplayRole) const override;
    private:
        QLocale loc;
    };
}

OrderViewDialog::OrderViewDialog(int cs_id, QWidget* parent)
: customerName(), QDialog(parent) {
    QSqlQuery q;
    q.prepare("SELECT name FROM customers WHERE id = ?");
    q.addBindValue(cs_id);
    q.exec() && q.next();
    
    setWindowTitle(QString("Data order | %1").arg(q.value(0).toString()));
    tableView = new QTableView(this);
    tableView->setObjectName("tableView");
    model = new QSqlQueryModel(this);
    model->setObjectName("model");
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    QLabel* labelCari = new QLabel("Cari :", this);
    QLineEdit* editCari = new QLineEdit(this);
    editCari->setObjectName("editCari");
    editCari->setClearButtonEnabled(true);
    QHBoxLayout* pl = new QHBoxLayout;
    pl->addWidget(labelCari);
    pl->addWidget(editCari);
    pl->addStretch(1);
    
    QVBoxLayout* ml = new QVBoxLayout;
    ml->addLayout(pl, 0);
    ml->addWidget(tableView, 1);
    
    setLayout(ml);
    
    q.prepare(R"--(
SELECT  od.order_id AS [OrderID],
        od.order_date AS [Tanggal],
        od.name AS [Nama],
        pr.name AS [Produk],
        od.quantity AS [Qty],
        ad.ad_price AS [SubTotal]
        FROM orders od JOIN products pr ON od.product_id = pr.product_id
                       JOIN orders_calc ad ON ad.order_id = od.order_id
                       JOIN customers cs ON od.customer_id = cs.customer_id
                       WHERE cs.customer_id = ? AND od.invoice_id IS NULL AND od.status = 'OK';
    )--");
    q.addBindValue(cs_id);
    q.exec();
    model->setQuery(q);
    
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    tableView->verticalHeader()->setMinimumSectionSize(15);
    tableView->verticalHeader()->setDefaultSectionSize(18);
    setMinimumSize(380, 200);
    
    Viewer::_Proxy* proxy = new Viewer::_Proxy(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(-1);
    tableView->setModel(proxy);
    tableView->hideColumn(0);
    tableView->resizeColumnsToContents();
    tableView->setSortingEnabled(true);
    tableView->sortByColumn(-1, Qt::AscendingOrder);
    tableView->setSizeAdjustPolicy(tableView->AdjustToContents);
    tableView->setSelectionBehavior(tableView->SelectRows);
    adjustSize();
    
    connect(tableView, &QWidget::customContextMenuRequested, this, &OrderViewDialog::show_tableView_contextMenu);
    connect(editCari, &QLineEdit::textEdited, [this, proxy](QString t) {proxy->setFilterFixedString(t);});
}

void OrderViewDialog::show_tableView_contextMenu(const QPoint& pt) {
    auto smod = tableView->selectionModel();
    auto showPoint = tableView->viewport()->mapToGlobal(pt);
    QMenu context;
    auto bayar = context.addAction("Bayar");
    auto edit = context.addAction("Edit");
    QModelIndexList mil = smod->selectedRows(0);
    
    if(mil.count() > 1) {
        edit->setDisabled(true);
    }
    
    QAction* act = context.exec(showPoint);
    if(!act) return;
    if(act == bayar) {
        QList<int> orders;
        for(auto i = mil.cbegin(); i != mil.cend(); ++i) {
            orders << (*i).data(Qt::EditRole).toInt();
        }
        emit createInvoiceForOrders(orders);
    } else if(act == edit) {
        emit editOrder(mil.at(0).data(Qt::EditRole).toInt());
    }
}

OrderViewDialog::~OrderViewDialog() {}

void OrderViewDialog::reloadData() {
    auto q = model->query();
    q.exec();
    model->setQuery(q);
}

QVariant Viewer::_Proxy::data(const QModelIndex& mi, int role) const {
    if(role == Qt::DisplayRole) {
        switch (mi.column()) {
            case 4:
            case 5:
                return loc.toString(mi.data(Qt::EditRole).toInt());
            case 1:
                return loc.toString(QDate::fromString(mi.data(Qt::EditRole).toString(), "yyyy-MM-dd"), "dd MMMM yyyy");
        }
    } else if(role == Qt::TextAlignmentRole) {
        switch (mi.column()) {
            case 4:
            case 5:
                return int(Qt::AlignRight | Qt::AlignVCenter);
            case 1:
                return Qt::AlignCenter;
        }
    }
    return QSortFilterProxyModel::data(mi, role);
}