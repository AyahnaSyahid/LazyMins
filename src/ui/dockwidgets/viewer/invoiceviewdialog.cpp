#include "invoiceviewdialog.h"
#include "ui_files/ui_invoiceviewdialog.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSortFilterProxyModel>
#include <QDate>

namespace InvoiceViewDialogNS {
    class _Proxy : public QSortFilterProxyModel {
        QLocale loc;
    public:
        explicit _Proxy(QObject* parent=nullptr) : QSortFilterProxyModel(parent) {}
        ~_Proxy() = default;
        QVariant data(const QModelIndex&, int =Qt::EditRole) const override;
    };
}

InvoiceViewDialog::InvoiceViewDialog(int cid, QWidget* parent) :
ui(new Ui::InvoiceViewDialog), model(new QSqlQueryModel(this)), QDialog(parent) {
    ui->setupUi(this);
    
    QSqlQuery q;
    q.prepare(R"--(
        SELECT inv.customer_id AS CSID,
               invs.invoice_id AS INVID,
               invs.customer_name AS Konsumen,
               invs.invoice_date AS Tanggal,
               invs.invoice_code AS KODE,
               invs.unpaid AS Sisa
          FROM invoices_summary invs
               INNER JOIN
               invoices inv ON inv.invoice_id = invs.invoice_id
         WHERE invs.unpaid > 0 AND inv.customer_id = ?;
    )--");
    q.addBindValue(cid);
    q.exec();
    
    model->setQuery(q);
    
    auto proxy = new InvoiceViewDialogNS::_Proxy(this);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn(-1);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    
    connect(ui->invoiceFilter, SIGNAL(textEdited(QString)), proxy, SLOT(setFilterFixedString(QString)));
    
    ui->invoiceView->setModel(proxy);
    // ui->invoiceView->hideColumn(0);
    ui->invoiceView->hideColumn(1);
    ui->invoiceView->hideColumn(2);
};

InvoiceViewDialog::~InvoiceViewDialog() {
    delete ui;
}

QVariant InvoiceViewDialogNS::_Proxy::data(const QModelIndex& mi, int role) const {
    if(role == Qt::DisplayRole) {
        if(mi.column() == 5) {
            return loc.toString(mi.data(Qt::EditRole).toInt());
        } else if (mi.column() == 3) {
            return loc.toString(QDate::fromString(mi.data(Qt::EditRole).toString(), "yyyy-MM-dd"), "dd MMMM yyyy");
        }
    } else if (role == Qt::TextAlignmentRole) {
        if(mi.column() == 5) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        } else if (mi.column() == 3) {
            return Qt::AlignCenter;
        }
    }
    return QSortFilterProxyModel::data(mi, role);
}