#include "notamodel.h"
#include <QLocale>
#include <QChar>
#include <QDateTime>
#include <QtDebug>

NotaModel::NotaModel(QObject* parent) :
    pjs {}, QAbstractTableModel(parent) {}

NotaModel::~NotaModel() {
    pjs.clear();
}

QVariant NotaModel::data(const QModelIndex& mi, int role) const {
    if(!mi.isValid()) return QVariant();
    const QSqlRecord& record = pjs.at(mi.row());
    int column = mi.column();
    QLocale l(QLocale::Indonesian, QLocale::Indonesia);
    
    if(role == Qt::EditRole){
        return record.value(column);
    }
    if (role == Qt::TextAlignmentRole) {
        switch (column) {
            case  0: // id
            case  1: // konsumen
            // case 2: // atn
            case  3: // produk
            // case 4: // tanggal order
            // case 5: // nama file
            case  6: // panjang
            case  7: // tinggi
            case  8: // harga satuan
            case 10: // disc
            case 11: // harga jual
                return (int) (Qt::AlignRight | Qt::AlignVCenter);
            case  9: // qty
            case 12: // revisi
            case 13: // admin id
            case 14: // id pemb
                return (int) Qt::AlignCenter;
            default:
                return QVariant();
        }
    }
    if(role == Qt::DisplayRole) {
        switch (column) {
            case 4:
            case 12:
                return l.toString(QDateTime::fromString(record.value(column).toString(), "yyyy-MM-dd HH:mm:ss.zzz"), "dddd, d MMM yyyy");
            case 6:
            case 7:
            case 8:
            case 9:
            case 11:
            case 10:
                return l.toString(record.value(column).toDouble(), 'g', 9);
            default:
                return record.value(column);
        }
    }
    return QVariant();
}

QVariant NotaModel::headerData(int sec, Qt::Orientation orient, int role) const {
        
        static QStringList _header { "ID", "Konsumen ID", "ATN", "Produk ID", "Tgl. Order", "Nama File", "Panjang", "Tinggi", "Harga Satuan",
                                     "Qty", "Disc Rp", "Harga Jual", "Tgl. Revisi", "Admin ID", "ID Pembayaran" };
        if(pjs.count() && orient == Qt::Horizontal && role == Qt::DisplayRole) {
            return _header[sec];
        }
        return QAbstractTableModel::headerData(sec, orient, role);
}

void NotaModel::pushRecord(const QSqlRecord& pj) {
    int curow = pjs.count();
    if(!curow) {
        beginResetModel();
        pjs.append(pj);
        endResetModel();
    } else {
        beginInsertRows(QModelIndex(), curow, curow);
        pjs.append(pj);
        endInsertRows();
    }
}

void NotaModel::pushRecord(const QVector<QSqlRecord>& vpj) {
    int curow = rowCount();
    beginInsertRows(QModelIndex(), curow, curow + vpj.count());
    pjs += vpj;
    endInsertRows();
}

QSqlRecord NotaModel::pullRecord(int row) {
    QSqlRecord p;
    beginRemoveRows(QModelIndex(), row, row);
    p = pjs.takeAt(row);
    endRemoveRows();
    if(pjs.count() < 1) {
        beginResetModel();
        endResetModel();
    }
    return p;
}

bool  NotaModel::hasRecord(const QSqlRecord& rc) const {
    for(auto ie=pjs.constBegin() ; ie != pjs.constEnd() ; ++ie) {
        if(rc == *ie) return true;
    }
    return false;
}