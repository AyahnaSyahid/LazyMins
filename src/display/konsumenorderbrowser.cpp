#include "konsumenorderbrowser.h"

#include "src/display/ui_konsumenorderbrowser.h"

#include "src/managers/basemanager.h"
#include "src/managers/konsumenmanager.h"
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QSqlQueryModel>
#include <QScrollBar>
#include <QStyledItemDelegate>

namespace
{
    const QString SELECT_QUERY = R"-(
SELECT datetime(o.order_date, 'localtime'),
       oi.product_name,
       oi.quantity,
       oi.sale_price,
       oi.subtotal,
       oi.finishing_total,
       oi.total,
       oi.notes,
       inv.settlement_status
  FROM order_items oi
       JOIN orders o ON o.id = oi.order_id
       JOIN konsumen k ON k.id = o.customer_id
       JOIN invoices inv ON inv.id = o.invoice_id
 WHERE k.id = :kid 
   AND o.staging_status IS NOT NULL 
   AND o.staging_status <> 'cancelled'
 ORDER BY o.order_date DESC
    )-";

    const QString MinMaxDate = R"-(
SELECT MIN(DATE(o.order_date, 'localtime')) AS min_date,
       MAX(DATE(o.order_date, 'localtime')) AS max_date
  FROM orders o
 WHERE o.customer_id = :kid 
   AND o.staging_status IS NOT NULL 
   AND o.staging_status <> 'cancelled';
    )-";

    class OrderBrowserDelegate : public QStyledItemDelegate
    {
    public:
        OrderBrowserDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
        OrderBrowserDelegate() {}

    protected:
        void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex &ix) const override
        {
            QStyledItemDelegate::initStyleOption(opt, ix);
            switch (ix.column())
            {
            case 0:
            case 8:
                opt->displayAlignment = Qt::AlignCenter;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                opt->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                opt->text = QLocale().toString(ix.data().toLongLong());
            }
            if (ix.column() == 8)
            {
                auto data = ix.data().toString();
                opt->text = OrderBrowserDelegate::statusMap.value(data, data);
            }
        }

    private:
        static QMap<QString, QString> statusMap;
    };

    class OrderBrowserSortModel : public QSortFilterProxyModel
    {
    public:
        OrderBrowserSortModel(QObject *parent) : QSortFilterProxyModel(parent) {}
        ~OrderBrowserSortModel() {}

        QVariant data(const QModelIndex &ix, int role) const override
        {
            if (role == Qt::ToolTipRole && ix.column() == 7)
            {
                return ix.data(Qt::DisplayRole).toString();
            }
            return QSortFilterProxyModel::data(ix, role);
        }
    };

    QMap<QString, QString> OrderBrowserDelegate::statusMap{
        {"paid", "Lunas"},
        {"unpaid", "Belum bayar"},
        {"partial", "Belum lunas"},
        {"refunded", "Dikembalikan"}};

}

KonsumenOrderBrowser::KonsumenOrderBrowser(QWidget *parent) : QDialog(parent),
                                                              ui(new Ui::KonsumenOrderBrowser),
                                                              model(new QSqlQueryModel(this)),
                                                              proxy(new OrderBrowserSortModel(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(proxy);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(1);
}

KonsumenOrderBrowser::~KonsumenOrderBrowser() { delete ui; }

bool KonsumenOrderBrowser::setCustomerId(int KID)
{
    KonsumenManager km;
    auto optkon = km.getById(KID);
    if (!optkon.has_value())
        return false;
    QString textLabel = ui->label->text().arg(optkon->value("nama_lengkap").toString());
    ui->label->setText(textLabel);
    
    QSqlQuery mm(BaseManager::connection);
    mm.prepare(MinMaxDate);
    mm.bindValue(":kid", KID);
    if ( mm.exec() && mm.next() ) {
        minMax = { mm.value(0).toDate(), mm.value(1).toDate() };
    }

    if (minMax.isEmpty()) {
        return false;
    } else if (minMax.count() == 1) {
        ui->dateEdit->setDate(minMax[0]);
        ui->dateEdit->setEnabled(false);
        ui->dateEdit_2->setDate(minMax[0]);
        ui->dateEdit_2->setEnabled(false);
    }

    QSqlQuery q(BaseManager::connection);
    q.prepare(SELECT_QUERY);
    q.bindValue(":kid", KID);
    q.exec();
    model->setQuery(std::move(q));
    if (model->rowCount() == 0)
        return false;

    setupHeaderData();
    setupItemDelegate();
    setupFilterConnection();
    

    finalizeUi();
    return true;
}

void KonsumenOrderBrowser::setupHeaderData()
{
    proxy->setHeaderData(0, Qt::Horizontal, "Tanggal", Qt::DisplayRole);
    proxy->setHeaderData(1, Qt::Horizontal, "Nama", Qt::DisplayRole);
    proxy->setHeaderData(2, Qt::Horizontal, "Jumlah", Qt::DisplayRole);
    proxy->setHeaderData(3, Qt::Horizontal, "Satuan", Qt::DisplayRole);
    proxy->setHeaderData(3, Qt::Horizontal, "Harga satuan", Qt::ToolTipRole);
    proxy->setHeaderData(4, Qt::Horizontal, "Subtotal", Qt::DisplayRole);
    proxy->setHeaderData(5, Qt::Horizontal, "Finishing", Qt::DisplayRole);
    proxy->setHeaderData(5, Qt::Horizontal, "Harga finishing", Qt::ToolTipRole);
    proxy->setHeaderData(6, Qt::Horizontal, "Total", Qt::DisplayRole);
    proxy->setHeaderData(7, Qt::Horizontal, "Catatan", Qt::DisplayRole);
    proxy->setHeaderData(8, Qt::Horizontal, "Pembayaran", Qt::DisplayRole);
}

void KonsumenOrderBrowser::setupItemDelegate()
{
    auto idg = new OrderBrowserDelegate(this);
    ui->tableView->setItemDelegate(idg);
}

void KonsumenOrderBrowser::setupFilterConnection()
{
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->lineEdit, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);
}

void KonsumenOrderBrowser::finalizeUi()
{
    ui->tableView->resizeColumnsToContents();
    auto vScroll = ui->tableView->verticalScrollBar();
    auto hHeader = ui->tableView->horizontalHeader();
    int addition = 2;
    ui->tableView->setMinimumWidth(addition + hHeader->length() + (vScroll->isVisible() ? vScroll->width() : 0));

    if(minMax.count() == 2) {
        ui->dateEdit->setMinimumDate(minMax[0]);
        ui->dateEdit->setDate(minMax[0]);
        ui->dateEdit->setMaximumDate(minMax[1].addDays(-1));
        ui->dateEdit_2->setMinimumDate(minMax[0].addDays(1));
        ui->dateEdit_2->setMaximumDate(minMax[1]);
    }
}
