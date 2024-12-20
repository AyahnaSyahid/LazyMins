#include "productsdockwidget.h"
#include "ui_productsdockwidget.h"
#include "userpermissions.h"
#include "csvproductupdaterdialog.h"
#include "databasenotifier.h"
#include "helper.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSortFilterProxyModel>
#include <QDateTime>
#include <QComboBox>
#include <QVariant>
#include <QPushButton>
#include <QtMath>
#include <QtDebug>

class ProductsProxy : public QSortFilterProxyModel {
public:
    ProductsProxy(QObject* parent=nullptr);
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const override;
};

ProductsDockWidget::ProductsDockWidget(QWidget* parent)
: _limit(100), ui(new Ui::ProductsDockWidget), QDockWidget(parent) {
    ui->setupUi(this);
    this->addAction(ui->actionCSVUpdate);
    auto mainModel = new QSqlQueryModel(this);
    auto proxy = new ProductsProxy(this);
    mainModel->setObjectName("mainModel");
    mainModel->setQuery(baseQuery());
    proxy->setSourceModel(mainModel);
    ui->tableView->setModel(proxy);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(5);
    
    connect(ui->firstButton, &QPushButton::clicked, [this]() {
        if(_currentPage > 1) {
            _currentPage = 1;
            refreshModel();
        }
    });
    
    connect(ui->prevButton, &QPushButton::clicked, [this]() {
        if(_currentPage > 1) {
            _currentPage--;
        }
        refreshModel();
    });
    
    connect(ui->nextButton, &QPushButton::clicked, [this]() {
        if(_currentPage < _totalPage) {
            _currentPage++;
        }
        refreshModel();
    });
    
    connect(ui->lastButton, &QPushButton::clicked, [this]() {
        if(_currentPage != _totalPage) {
            _currentPage = _totalPage;
        }
        refreshModel();
    });
    
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProductsDockWidget::refreshModel);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &ProductsDockWidget::refreshModel);
    connect(dbNot, DatabaseNotifier::tableChanged, [this](QString tname) {
        if(tname == "products" || tname == "orders") refreshModel();
    }); 
    refreshModel();
}

ProductsDockWidget::~ProductsDockWidget(){
    delete ui;
}

void ProductsDockWidget::refreshModel(){
    auto mainModel = findChild<QSqlQueryModel*>("mainModel");
    QSqlQuery q(QString("SELECT COUNT(*) FROM ( %1 %2 )").arg(baseQuery(), whereQuery()));
    if(q.lastError().isValid()) {
        qDebug() << q.lastError().text();
        qDebug() << q.executedQuery();
        return ;
    }
    q.next();
    auto curTotal = qCeil(q.value(0).toDouble() / _limit);
    _currentPage = qBound<qint64>(1, _currentPage, _totalPage);
    
    ui->spinBox->blockSignals(true);
    if(curTotal != _totalPage) {
        _totalPage = curTotal;
        ui->spinBox->setMaximum(_totalPage);
        if(_currentPage > curTotal) {
            ui->spinBox->setValue(_currentPage);
        }
        ui->spinBox->setSuffix(QString(" / %1").arg(_totalPage));
    }
    if(ui->spinBox->value() != _currentPage) {
        ui->spinBox->setValue(_currentPage);
    }
    ui->spinBox->blockSignals(false);
    
    ui->navPaging->setHidden(_totalPage < 2);
    ui->firstButton->setEnabled(_currentPage != 1);
    ui->prevButton->setEnabled(_currentPage != 1);
    ui->nextButton->setEnabled(_currentPage != _totalPage);
    ui->lastButton->setEnabled(_currentPage != _totalPage);
    
    QStringList nq { baseQuery() };
    auto where = whereQuery();
    if(!where.isEmpty()) {
       nq << where;
    }
    nq << limitQuery();
    mainModel->setQuery(nq.join(" "));
    ui->tableView->resizeColumnsToContents();
}

QString ProductsDockWidget::baseQuery() const {
    QString base( R"-(
SELECT product_id AS ProdID,
       code AS Kode,
       Transaksi,
       Terjual,
       CASE WHEN use_area THEN uk ELSE NULL END AS Ukuran
  FROM products,
       (SELECT product_id,
               COUNT(order_id) AS Transaksi,
               SUM(quantity) AS Terjual,
               SUM( (width_mm / 100.0 * height_mm / 100.0) * quantity / 100.0) AS uk
          FROM orders
         %1
         GROUP BY orders.product_id)
       AS OO USING(product_id) 
    )-" );
    auto ci = ui->comboBox->currentIndex();
    switch (ci) {
        case 0:
            return base.arg(" WHERE date(order_date) = date('now') ");
        case 1:
            return base.arg(" WHERE strftime('%Y-%W', order_date) = strftime('%Y-%W', datetime('now')) ");
        case 2:
            return base.arg(" WHERE strftime('%Y-%m', order_date) = strftime('%Y-%m', datetime('now')) ");
        case 3:
            return base.arg(" WHERE strftime('%Y', order_date) = strftime('%Y', datetime('now')) ");
        default:
            return base.arg(" ");
    }
}

QString ProductsDockWidget::limitQuery() const {
    return QString("LIMIT %1 OFFSET %2 ").arg(_limit).arg((_currentPage - 1) * _limit);
}

QString ProductsDockWidget::whereQuery() const {
    QString sp;
    auto findName = ui->lineEdit->text().trimmed();
    if(!findName.isEmpty()) {
        sp = QString("code LIKE '%%1%'").arg(findName);
    }
    if(sp.isEmpty()) return sp;
    return QString(" WHERE %1 ").arg(sp);
}

void ProductsDockWidget::on_actionCSVUpdate_triggered(){
    if(!UserPermissions::hasPermission(PermissionItem("ModifyProduct"))) {
        MessageHelper::warning(nullptr, "Peringatan", "Anda tidak memiliki hak untuk mengatur Produk");
        return;
    }
    auto csu = new CSVProductUpdaterDialog(this);
    csu->setAttribute(Qt::WA_DeleteOnClose);
    connect(csu, &QDialog::accepted, [this]() {
        emit productsUpdated();
    });
    csu->open();
}

ProductsProxy::ProductsProxy(QObject* parent) : QSortFilterProxyModel(parent) {}

QVariant ProductsProxy::data(const QModelIndex& mi, int role) const {
    auto def = QSortFilterProxyModel::data(mi, role);
    auto cn = headerData(mi.column(), Qt::Horizontal, Qt::DisplayRole).toString();
    QLocale loc;
    if(role == Qt::DisplayRole) {
        if(cn == "Transaksi" || cn == "Terjual") {
            loc.toString(def.toLongLong());
        } else if (cn == "Ukuran") {
            // qint64 vd = def.toDouble() * 100;
            loc.toString(def.toDouble(), 'g', 12);
        }
    }
    if(role == Qt::TextAlignmentRole) {
        if(cn == "ProdID" || cn == "Transaksi" || cn == "Terjual" || cn == "Ukuran") {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    return def;
}