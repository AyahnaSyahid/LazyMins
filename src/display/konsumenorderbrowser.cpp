#include "konsumenorderbrowser.h"

#include "src/display/ui_konsumenorderbrowser.h"

#include "src/managers/basemanager.h"
#include "src/managers/konsumenmanager.h"
#include <QHeaderView>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QSqlQueryModel>
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
    m_customerId = KID;
    KonsumenManager km;
    auto optkon = km.getById(KID);
    if (!optkon.has_value())
        return false;
    QString textLabel = ui->label->text().arg(optkon->value("nama_lengkap").toString());
    ui->label->setText(textLabel);

    initializeMinMax(KID);

    if (minMax.isEmpty())
    {
        return false;
    }

    setupDateEdit();
    
    finalizeUi();

    updateQuery();
    
    if (model->rowCount() == 0)
        return false;

    setupHeaderData();
    setupItemDelegate();

    setupFilterConnection();
    ui->tableView->resizeColumnsToContents();
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
    connect(ui->minDateEdit, &QDateEdit::dateChanged, this, &KonsumenOrderBrowser::updateQuery);
    connect(ui->maxDateEdit, &QDateEdit::dateChanged, this, &KonsumenOrderBrowser::updateQuery);
}

void KonsumenOrderBrowser::setupDateEdit()
{
    QList<QDateEdit *> dateEdits{ui->minDateEdit, ui->maxDateEdit};
    auto minMaxUnavailable = minMax.count() != 2;

    for (auto e : dateEdits)
    {
        e->setDisabled(minMaxUnavailable);
        e->setToolTip(minMaxUnavailable ? "Tidak dapat menetapkan filter tanggal" : "");
    }
}

void KonsumenOrderBrowser::finalizeUi()
{
    auto vScroll = ui->tableView->verticalScrollBar();
    auto hHeader = ui->tableView->horizontalHeader();
    int addition = 2;
    ui->tableView->setMinimumWidth(addition + hHeader->length() + (vScroll->isVisible() ? vScroll->width() : 0));
    if (minMax.count() == 2)
    {
        ui->minDateEdit->blockSignals(true);
        ui->maxDateEdit->blockSignals(true);

        QDate minDate = minMax[0];
        QDate maxDate = minMax[1];
        QDate minPlus1 = minDate.addDays(1);
        QDate maxMinus1 = maxDate.addDays(-1);
        QDate maxPlus1 = maxDate.addDays(1);
        ui->minDateEdit->setMinimumDate(minDate);
        ui->minDateEdit->setDate(minDate);
        ui->minDateEdit->setMaximumDate(maxMinus1);
        ui->maxDateEdit->setMinimumDate(minPlus1);
        ui->maxDateEdit->setMaximumDate(maxPlus1);
        ui->maxDateEdit->setDate(maxPlus1);

        ui->minDateEdit->blockSignals(false);
        ui->maxDateEdit->blockSignals(false);
    }
}

void KonsumenOrderBrowser::initializeMinMax(int KID)
{
    QSqlQuery mm(BaseManager::connection);
    mm.prepare(MinMaxDate);
    mm.bindValue(":kid", KID);
    if (mm.exec() && mm.next())
    {
        minMax = {mm.value(0).toDate(), mm.value(1).toDate()};
    }
}

QString KonsumenOrderBrowser::buildQuery() const
{
    if (minMax.isEmpty()) return SELECT_QUERY;
    if (minMax[0] == minMax[1]) return SELECT_QUERY;
    
    QString query = SELECT_QUERY;
    QString dateCondition = "\n   AND DATE(o.order_date, 'localtime') BETWEEN :min_date AND :max_date\n ";
    query.replace("ORDER BY", dateCondition + "ORDER BY");
    // qDebug() << "[DEBUG] << " << query;
    return query;
}

void KonsumenOrderBrowser::updateQuery()
{
    if (m_customerId == 0) return;

    QSqlQuery q(BaseManager::connection);
    q.prepare(buildQuery());
    q.bindValue(":kid", m_customerId);

    if (minMax.count() == 2 && minMax[0] != minMax[1])
    {
        q.bindValue(":min_date", ui->minDateEdit->date().toString("yyyy-MM-dd"));
        q.bindValue(":max_date", ui->maxDateEdit->date().toString("yyyy-MM-dd"));
    }

    q.exec();
    model->setQuery(std::move(q));
}
