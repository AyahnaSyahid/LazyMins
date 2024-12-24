#include "managernotadockwidget.h"
#include "ui_managernotadockwidget.h"
#include "invoiceitem.h"
#include "useritem.h"
#include "userpermissions.h"
#include "tabelrepayment.h"
#include "notifier.h"
#include "paymentdialog2.h"

#include <QSqlQuery>
#include <QDialog>
#include <QMenu>
#include <QAction>
#include <QCheckBox>
#include <QToolButton>
#include <QModelIndex>
#include <QVariant>
#include <QLocale>
#include <QtMath>
#include <QDateTime>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>

class ModelAdapter : public QSortFilterProxyModel {
public:
    ModelAdapter(QObject * =nullptr);
    ~ModelAdapter();
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const;
};

ManagerNotaDockWidget::ManagerNotaDockWidget(QWidget* parent)
: ui(new Ui::ManagerNotaDockWidget), _currentPage(1), _limit(100), QDockWidget(parent){
    ui->setupUi(this);
    invoiceModel = new QSqlQueryModel(this);
    auto proxy = new ModelAdapter(this);
    proxy->setSourceModel(invoiceModel);
    ui->tableView->setModel(proxy);
    connect(ui->checkBox, &QCheckBox::toggled, this, &ManagerNotaDockWidget::applyFiltering);
    connect(dbNot, &DatabaseNotifier::tableChanged, this, &ManagerNotaDockWidget::onTableChanged);
    connect(ui->firstButton, &QToolButton::clicked, this, &ManagerNotaDockWidget::navButtonHandler);
    connect(ui->prevButton, &QToolButton::clicked, this, &ManagerNotaDockWidget::navButtonHandler);
    connect(ui->nextButton, &QToolButton::clicked, this, &ManagerNotaDockWidget::navButtonHandler);
    connect(ui->lastButton, &QToolButton::clicked, this, &ManagerNotaDockWidget::navButtonHandler);
    applyFiltering();
    ui->tableView->resizeColumnsToContents();
}

ManagerNotaDockWidget::~ManagerNotaDockWidget() {
    delete ui;
}

QString ManagerNotaDockWidget::baseQuery() const {
    return { R"-(
SELECT inv.invoice_id AS INVID,
       inv.physical_iid AS NotaFisik,
       cus.customer_name AS Konsumen,
       inv.total_amount AS Nilai,
       inv.total_paid AS Terbayar,
       inv.discount_amount AS Diskon,
       inv.total_amount - COALESCE(inv.total_paid + inv.discount_amount, 0) AS Sisa,
       inv.due_date AS [Jadwal Tagihan]
  FROM invoices inv
       JOIN
       customers cus USING(customer_id)
    )-" };
}

QString ManagerNotaDockWidget::whereClause() const {
    auto fst = ui->lineEdit->text().trimmed();
    auto sln = ui->checkBox->isChecked();
    QStringList flist;
    if(!fst.isEmpty()) {
        flist << QString("COALESCE(NotaFisik, '') || ' ' || INVID || ' ' || Konsumen LIKE '%%1%'").arg(fst);
    }
    if(!sln) {
        flist << QString("Sisa > 0");
    }
    if (flist.count()) {
        return flist.join(" AND ");
    }
    return "";
}

void ManagerNotaDockWidget::applyFiltering() {
    int invc = invoicesCount();
    auto im = qobject_cast<QSqlQueryModel*>(invoiceModel);
    QStringList fql;
    fql << baseQuery();
    auto wq = whereClause();
    if(!wq.isEmpty()) {
        fql << QString("WHERE %1").arg(wq);
    }
    fql << limitClause();
    im->setQuery(fql.join(" "));
    auto hidePaging = invc < _limit;
    ui->navPaging->setHidden(hidePaging);
    if(!hidePaging) {
        auto mPage = qCeil(invc / _limit);
        ui->spinBox->setSuffix(QString("/%1").arg(mPage));
        ui->spinBox->setMaximum(mPage);
        ui->spinBox->setValue(_currentPage);
    }
}

void ManagerNotaDockWidget::navButtonHandler() {
    auto sd = qobject_cast<QToolButton*>(sender());
    if(!sd) return;
    auto mPage = ui->spinBox->maximum();
    if(sd == ui->firstButton) {
        if(_currentPage > 1) {
            _currentPage -= 1;
            applyFiltering();
        }
    } else if (sd == ui->prevButton) {
        if(_currentPage > 1) {
            _currentPage = 1;
            applyFiltering();
        }
    } else if (sd == ui->nextButton) {
        if (_currentPage < mPage) {
            _currentPage += 1;
            applyFiltering();
        }
    } else if ( sd == ui->lastButton) {
        if (_currentPage < mPage) {
            _currentPage = mPage;
            applyFiltering();
        }
    }
}

QString ManagerNotaDockWidget::limitClause() const {
    return QString(" LIMIT %1 OFFSET %2").arg(_limit).arg((_currentPage - 1) * _limit);
}

void ManagerNotaDockWidget::on_lineEdit_textChanged(const QString &txt) {
    applyFiltering();
}

qint64 ManagerNotaDockWidget::invoicesCount() const {
    QSqlQuery q;
    auto wc = whereClause();
    QString gQue("SELECT COUNT(invoice_id) FROM invoices");
    if (!wc.isEmpty()) {
        gQue += QString(" WHERE %1").arg(wc);
    }
    q.exec("BEGIN TRANSACTION");
    q.prepare(gQue);
    if(!(q.exec() && q.next())) {
        q.exec("ROLLBACK");
        return 0;
    }
    auto rv = q.value(0).toLongLong();
    q.exec("ROLLBACK");
    return rv;
}

void ManagerNotaDockWidget::onTableChanged(const QString &st) {
    QStringList hand {"orders", "invoices", "invoice_payments", "customers"};
    if(hand.contains(st)) {
        applyFiltering();
    }
}

void ManagerNotaDockWidget::on_tableView_customContextMenuRequested(const QPoint& pp) {
    auto gp = ui->tableView->viewport()->mapToGlobal(pp);
    auto smod = ui->tableView->selectionModel();
    auto ci = smod->currentIndex();
    auto cu = LoginNotifier::currentUser();
    if(!ci.isValid()) return;
    auto nid = ci.siblingAtColumn(0).data(Qt::EditRole).toLongLong();
    auto vp = UserPermissions::hasPermission(PermissionItem("Viewer"));
    auto mip = UserPermissions::hasPermission(PermissionItem("ManageInvoices"));
    QMenu ctm(this);
    auto vie = ctm.addAction("Lihat Pembayaran");
    auto edt = ctm.addAction("Edit Nota");
    auto sep = ctm.addSeparator();
    auto del = ctm.addAction("Hapus");
    
    vie->setEnabled(vp);
    edt->setEnabled(mip);
    
    auto sel = ctm.exec(gp);
    
    if(sel == vie) {
        auto dialog = new QDialog(this);
        auto tarep = new TabelRepayment(dialog);
        dialog->setObjectName("TabelRepaymentDialog");
        dialog->setWindowTitle("Data Pembayaran Invoice");
        tarep->setNota(nid);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setLayout(tarep->layout());
        dialog->adjustSize();
        dialog->open();
    } else if ( sel == edt) {
        auto dia = new PaymentDialog2(this);
        dia->setAttribute(Qt::WA_DeleteOnClose);
        dia->setPayment(nid);
        dia->open();
    }
}

ModelAdapter::ModelAdapter(QObject *parent)
:  QSortFilterProxyModel(parent) {}

ModelAdapter::~ModelAdapter(){}

QVariant ModelAdapter::data(const QModelIndex& mi, int role) const {
    auto def = QSortFilterProxyModel::data(mi, role);
    auto loc = QLocale();
    if(role == Qt::TextAlignmentRole) {
        switch (mi.column()) {
            case 0:
            case 3:
            case 4:
            case 5:
            case 6:
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            case 1:
            case 7:
                return Qt::AlignCenter;
        }
    } else if (role == Qt::DisplayRole) {
        switch (mi.column()) {
            case 3:
            case 4:
            case 5:
            case 6:
                return loc.toString(def.toLongLong());
            case 7:
                return loc.toString(def.toDateTime().toLocalTime(), "dd/MM/yyyy HH:mm");
        }
    }
    return def;
}