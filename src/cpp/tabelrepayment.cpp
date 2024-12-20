#include "tabelrepayment.h"
#include "ui_tabelrepayment.h"
#include "notifier.h"
#include "itemdelegates.h"
#include "invoiceitem.h"
#include "invoicepaymentsitem.h"
#include "helper.h"
#include "editrepaymentdialog.h"
#include "addrepaymentdialog.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QSqlQuery>
#include <QCheckBox>
#include <QDialog>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSpinBox>
#include <QSqlError>
#include <QtDebug>

class Adapter : public QSortFilterProxyModel {
    QLocale loc;
public:
    Adapter(QObject* parent=nullptr);
    ~Adapter();
    QVariant data(const QModelIndex& mi, int role = Qt::DisplayRole) const;
};

TabelRepayment::TabelRepayment(QWidget* parent)
  : ui(new Ui::TabelRepayment), QWidget(parent)
{
    ui->setupUi(this);
    auto tabel = ui->tableView;
    auto m = new QSqlQueryModel(this);
    m->setObjectName("queryModel");
    auto proxy = new Adapter(this);
    proxy->setSourceModel(m);
    ui->tableView->setModel(proxy);
    setProperty("currentNotaId", -1);
    connect(dbNot, &DatabaseNotifier::tableChanged, this, &TabelRepayment::initRepayModel);
    // ui->tableView->hideColumn(0);
    // ui->tableView->hideColumn(1);
    // ui->tableView->hideColumn(5);
    // auto lh = new LihatHari;
    // connect(this, SIGNAL(destroyed()), lh, SLOT(deleteLater()));
    // ui->tableView->setItemDelegateForColumn(4, lh);
    // ui->tableView->horizontalHeader()->moveSection(4, 2);
    // ui->tableView->resizeColumnsToContents();
    // for(auto bt : findChildren<QPushButton*>()) {
        // bt->setDefault(false);
        // bt->setAutoDefault(false);
    // }
}

TabelRepayment::~TabelRepayment()
{
    delete ui;
}

void TabelRepayment::on_tableView_doubleClicked(const QModelIndex& mi)
{
    // if(!mi.isValid()) return;
    // auto repMod = qobject_cast<RepaymentModel*>(model());
    // auto before = repMod->record(mi.row());
    // EditRepaymentDialog erd(before, this);
    // if(erd.exec() == QDialog::Accepted){
        // repMod->select();
        // setNota(ns._id);
    // }
}

void TabelRepayment::setNota(qint64 i)
{
    setProperty("currentNotaId", i);
    InvoiceItem ii(i);
    ii.updateTotal();
    ii.updatePaid();
    ui->label_5->setText(QString("%1").arg(ii.invoice_id, 8, 10, QChar('0')));
    ui->label_6->setText(locale().toString(ii.total_amount));
    ui->label_7->setText(locale().toString(ii.total_paid));
    ui->label_8->setText(locale().toString(ii.discount_amount));
    
    auto sisa = ii.total_amount - (ii.discount_amount + ii.total_paid);
    
    ui->label_10->setText(sisa == 0 ? "Lunas" : ( 
                            sisa > 1 ? locale().toString(sisa) : 
                                QString("Cashback %1").arg(locale().toString(sisa))));
    if(ui->label_10->text().contains("Lunas")) {
        ui->pushButton2->hide();
        ui->pushButton->hide();
    } else if(ui->label_10->text().contains("Cashback")) {
        ui->pushButton2->show();
        ui->pushButton->hide();
    } else {
        ui->pushButton2->hide();
        ui->pushButton->show();
    }
    initRepayModel();
}

void TabelRepayment::on_pushButton_clicked() //Tambah
{
    auto dia = new AddRepaymentDialog(this);
    InvoiceItem ii(property("currentNotaId").toLongLong());
    dia->setValue(ii.total_amount - (ii.discount_amount + ii.total_paid));
    dia->setAttribute(Qt::WA_DeleteOnClose);
    connect(dia, &QDialog::accepted, this, &TabelRepayment::onAcceptedRepayment);
    dia->open();
}

void TabelRepayment::on_pushButton2_clicked() // Cashback
{
    // CashbackDialog cb(this);
    // cb.setValue(-ns.rest);
    // if(cb.exec() == QDialog::Accepted) {
        // bool ok = RepaymentModel::insertRepayment(ns._id, -cb.value(), false);
        // if(!ok) {
            // MessageHelper::warning(this, "Kesalahan", "Data tidak berhasil di input");
            // return;
        // }
    // }
    // setNota(ns._id);
}

// Print Nota
void TabelRepayment::on_pushButton3_clicked() 
{
    
}

void TabelRepayment::initRepayModel() {
    auto pnid = property("currentNotaId");
    if(!pnid.isValid()) return;
    auto mod = findChild<QSqlQueryModel*>("queryModel");
    QString qq("SELECT payment_id AS [ID], payment_date AS Tanggal, amount AS Jumlah, method AS Metode FROM invoice_payments WHERE invoice_id = %1");
    mod->setQuery(qq.arg(pnid.toLongLong()));
    ui->tableView->resizeColumnsToContents();
    // adjustSize();
}

void TabelRepayment::onAcceptedRepayment() {
    auto dia = qobject_cast<AddRepaymentDialog*>(sender());
    if (!dia) return;
    auto spb = dia->findChild<QSpinBox*>("spinBox");
    auto chb = dia->findChild<QCheckBox*>("checkBox");
    if(!spb) return;
    InvoicePaymentsItem ipi;
    ipi.invoice_id = property("currentNotaId").toLongLong();
    ipi.amount = spb->value();
    ipi.method = chb->isChecked() ? "transfer" : "cash";
    ipi.created_by = LoginNotifier::currentUser().user_id;
    ipi.created_date = QDateTime::currentDateTime();
    ipi.payment_date = QDateTime::currentDateTime();
    if(!ipi.save()) {
        dia->open();
        return;
    }
    dbNot->emitChanged("invoices");
    dbNot->emitChanged("invoice_payments");
    dia->deleteLater();
    setNota(ipi.invoice_id);
}

Adapter::Adapter(QObject* parent) : QSortFilterProxyModel(parent) {}
Adapter::~Adapter(){}
QVariant Adapter::data(const QModelIndex& mi, int role) const {
    auto def = QSortFilterProxyModel::data(mi, role);
    if (role == Qt::TextAlignmentRole) {
        switch (mi.column()) {
            case 0:
            case 2:
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            case 3:
                return Qt::AlignCenter;
        }
    } else if (role == Qt::DisplayRole) {
        switch (mi.column()) {
            case 0:
                return QString("%1").arg(mi.data(Qt::EditRole).toLongLong(), 8, 10, QChar('0'));
            case 1:
                return loc.toString(mi.data(Qt::EditRole).toDateTime().toLocalTime(), "dddd, dd/MM/yyyy");
            case 2:
                return loc.toString(mi.data(Qt::EditRole).toLongLong());
            case 3:
                return def.toString().toCaseFolded();
        }
    }
    return def;
}