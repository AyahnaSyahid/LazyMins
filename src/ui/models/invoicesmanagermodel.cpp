#include "invoicesmanagermodel.h"
#include "database.h"

#include <QModelIndex>
#include <QSqlTableModel>
#include <QSqlError>
#include <QBrush>
#include <QColor>

#include <QtDebug>



InvoicesManagerModel::InvoicesManagerModel(Database* _d, QObject* parent) :
db(_d), QSortFilterProxyModel(parent) {
    QSqlTableModel* qModel = new QSqlTableModel(this);
    qModel->setObjectName("invoices_summary_model");
    qModel->setTable("invoices_summary");
    qModel->setFilter("unpaid > 0");
    qModel->select();
    setSourceModel(qModel);
    setHeaderData(0, Qt::Horizontal, "InvoiceID", Qt::DisplayRole);
    setHeaderData(1, Qt::Horizontal, "Kode", Qt::DisplayRole);
    setHeaderData(2, Qt::Horizontal, "Tanggal", Qt::DisplayRole);
    setHeaderData(3, Qt::Horizontal, "Konsumen", Qt::DisplayRole);
    setHeaderData(4, Qt::Horizontal, "Total", Qt::DisplayRole);
    setHeaderData(5, Qt::Horizontal, "Terbayar", Qt::DisplayRole);
    setHeaderData(6, Qt::Horizontal, "Sisa", Qt::DisplayRole);
    setHeaderData(7, Qt::Horizontal, "Jumlah Pembayaran", Qt::DisplayRole);
    setHeaderData(8, Qt::Horizontal, "Harga Order", Qt::DisplayRole);
    setHeaderData(9, Qt::Horizontal, "Nilai Modal", Qt::DisplayRole);
    setHeaderData(10, Qt::Horizontal, "Jumlah Order", Qt::DisplayRole);
    setHeaderData(11, Qt::Horizontal, "Diskon", Qt::DisplayRole);
    setHeaderData(12, Qt::Horizontal, "Pembayaran Terakhir", Qt::DisplayRole);
}

InvoicesManagerModel::~InvoicesManagerModel() {}

QVariant InvoicesManagerModel::data(const QModelIndex& mi, int role) const {
    if(role == Qt::TextAlignmentRole) {
        switch (mi.column()) {
            case 0:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                return (int) (Qt::AlignRight | Qt::AlignVCenter);
            case 1:
            case 12:
                return Qt::AlignCenter;
            default :
                return QVariant();
        }
    } else if (role == Qt::DisplayRole) {
        switch (mi.column()) {
            case 0:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                return QLocale().toString(QSortFilterProxyModel::data(mi, Qt::EditRole).toInt());
        }
    } else if (role == Qt::BackgroundRole) {
        if(mi.siblingAtColumn(6).data(Qt::EditRole).toInt() == 0) {
            return QBrush(QColor(220, 255, 220));
        }
    }
    return QSortFilterProxyModel::data(mi, role);
}

void InvoicesManagerModel::showPaidInvoices(bool shw) {
    auto model = findChild<QSqlTableModel*>("invoices_summary_model");
    if(shw) {
        model->setFilter("");
    } else {
        model->setFilter("unpaid > 0");
    }
}

void InvoicesManagerModel::select() {
    auto model = findChild<QSqlTableModel*>("invoices_summary_model");
    if(model) {
        model->select();
    }
}