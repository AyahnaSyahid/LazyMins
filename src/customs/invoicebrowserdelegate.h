#pragma once

#include <QStyledItemDelegate>

class InvoiceBrowserDelegate : public QStyledItemDelegate
{
public:
    InvoiceBrowserDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
    void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(opt, index);
        int col = index.column();
        switch (col) { // alignment
            case  0:
            case  5:
            case  6:
            case  7:
            case  8:
            case  9:
            case 10:
            case 13:
            case 14:
                opt->displayAlignment = Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter);
                break;
            case  4:
            case 11:
            case 12:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                opt->displayAlignment = Qt::AlignCenter;
                break;
        }

        switch (col) {
            case  5:
            case  6:
            case  7:
            case  8:
            case  9:
            case 10:
                opt->text = QString("%L1").arg(index.data().toLongLong());
                break;
            case 13:
                opt->text = index.data().toInt() == 0 ? "Tidak ada" : index.data().toString();
                break;
            case 14:
                opt->text = index.data().isNull() ? "NO DATA" : index.data().toString();
            case 15:
                opt->text = index.data().toInt() == 0 ? "Tidak" : "Ya" ;
        }
    }

};
