#include "repaymentmodel.h"
#include "helper.h"
#include <QSqlQuery>
#include <QtDebug>

RepaymentModel::RepaymentModel(QObject* parent)
  : allowEdit(false), QSqlTableModel(parent)
{
    setTable("repayment");
    select();
    setHeaderData(0, Qt::Horizontal, "RP ID");
    setHeaderData(1, Qt::Horizontal, "NotaID");
    setHeaderData(2, Qt::Horizontal, "Cash");
    setHeaderData(3, Qt::Horizontal, "Transfer");
    setHeaderData(4, Qt::Horizontal, "Tanggal");
    setHeaderData(5, Qt::Horizontal, "Edit");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
}

RepaymentModel::~RepaymentModel()
{}

QVariant RepaymentModel::data(const QModelIndex& mi, int role) const
{
    if(!mi.isValid()) return QVariant();
    QVariant def = QSqlTableModel::data(mi, role);
    QLocale l;
    if(role == Qt::DisplayRole) {
        switch(mi.column()) {
            case 2:
            case 3:
                return l.toString(def.toDouble(), 'g', 9);
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch(mi.column()) {
            case 0:
            case 1:
            case 2:
            case 3:
                return (int) (Qt::AlignRight | Qt::AlignVCenter);
            case 4:
            case 5:
                return (int) Qt::AlignCenter;
        }
    }
    return def;
}

Qt::ItemFlags RepaymentModel::flags(const QModelIndex& i) const
{
    auto def = QSqlTableModel::flags(i);
    if(allowEdit) return def;
    return def ^ Qt::ItemIsEditable;
}

void RepaymentModel::setEditable(bool e)
{
    allowEdit = e;
}

bool RepaymentModel::insertRepayment(int paymentId, int value, bool trf)
{
    QSqlQuery q;
    q.exec("BEGIN TRANSACTION");
    q.prepare("INSERT into repayment (payment_id, cash, trf, ctime, mtime) VALUES (?, ?, ?, ?, ?)");
    auto ct = StringHelper::currentDateTimeString();
    q.addBindValue(paymentId);
    q.addBindValue(trf ? 0 : value);
    q.addBindValue(trf ? value : 0);
    q.addBindValue(ct);
    q.addBindValue(ct);
    q.exec();
    return q.exec("COMMIT");
}