#ifndef TmpSaleModel_H
#define TmpSaleModel_H

#include <QStandardItemModel>


class TmpSaleModel : public QStandardItemModel {
    Q_OBJECT
public:
    TmpSaleModel(QObject* = nullptr);
    ~TmpSaleModel();
    // QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex&, int) const;
    inline void clear() { setRowCount(0); }
    bool saveToSale();
    Qt::ItemFlags flags(const QModelIndex&) const override;
};


#endif