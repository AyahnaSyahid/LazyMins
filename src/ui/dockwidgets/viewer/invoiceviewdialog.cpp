#include "database.h"
#include "usermanager.h"
#include "invoiceviewdialog.h"
#include "ui_files/ui_invoiceviewdialog.h"
#include <QDate>
#include <QMenu>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>

namespace InvoiceViewDialogNS {
    class _Proxy : public QSortFilterProxyModel {
        QLocale loc;
    public:
        explicit _Proxy(QObject* parent=nullptr) : QSortFilterProxyModel(parent) {}
        ~_Proxy() = default;
        QVariant data(const QModelIndex&, int =Qt::EditRole) const override;
    };
}

InvoiceViewDialog::InvoiceViewDialog(int cid, Database* _d, QWidget* parent)
: ui(new Ui::InvoiceViewDialog), db(_d), model(new QSqlQueryModel(this)), QDialog(parent) {
  ui->setupUi(this);
  QSqlQuery q;
  q.prepare(R"--(
        SELECT inv.customer_id AS CSID,
               invs.invoice_id AS INVID,
               invs.name AS Konsumen,
               invs.invoice_date AS Tanggal,
               invs.invoice_code AS KODE,
               invs.unpaid AS Sisa
          FROM invoices_summary invs
               INNER JOIN
               invoices inv ON inv.invoice_id = invs.invoice_id
         WHERE invs.unpaid > 0 AND 
               inv.customer_id = ?;
      )--");
  q.addBindValue(cid);
  q.exec();
  
  model->setQuery(q);
  
  setWindowTitle(QString("Data Invoice | %1").arg(model->record(0).value("Konsumen").toString()));
  auto proxy = new InvoiceViewDialogNS::_Proxy(this);
  // proxy->setObjectName("proxyModel");
  proxy->setSourceModel(model);
  proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxy->setFilterKeyColumn(-1);
  
  connect(ui->invoiceFilter, SIGNAL(textEdited(QString)), proxy, SLOT(setFilterFixedString(QString)));
  
  ui->invoiceView->setModel(proxy);
  ui->invoiceView->hideColumn(0);
  ui->invoiceView->hideColumn(1);
  ui->invoiceView->hideColumn(2);
  ui->invoiceView->resizeColumnsToContents();
  
  connect(db->getTableModel("invoices"), &QAbstractItemModel::modelReset, this, &InvoiceViewDialog::reselectModel);
  connect(db->getTableModel("payments"), &QAbstractItemModel::modelReset, this, &InvoiceViewDialog::reselectModel);

};

InvoiceViewDialog::~InvoiceViewDialog() {
    delete ui;
}

void InvoiceViewDialog::reselectModel() {
  auto q = model->query();
  q.exec();
  model->setQuery(q);
}

void InvoiceViewDialog::on_invoiceView_customContextMenuRequested(const QPoint& p) {
  auto ix = ui->invoiceView->indexAt(p);
  if(!ix.isValid()) return;
  int invoiceId = ix.siblingAtColumn(1).data(Qt::EditRole).toInt();
  auto g_point = ui->invoiceView->viewport()->mapToGlobal(p);
  QMenu invoiceMenu("inv");
  auto actShow = invoiceMenu.addAction("Lihat");
  connect(actShow, &QAction::triggered, [this, &invoiceId] () { db->paymentRequest(invoiceId); });
  invoiceMenu.exec(g_point);
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

