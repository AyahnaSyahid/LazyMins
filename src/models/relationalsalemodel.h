#ifndef RelationalSaleModelH
#define RelationalSaleModelH

#include <QSqlRelationalTableModel>

class RelationalSaleModel : public QSqlRelationalTableModel
{
    Q_OBJECT
  public:
    RelationalSaleModel(QObject* =nullptr);
    ~RelationalSaleModel();
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const;
    QVariant idOfIndex(const QModelIndex& ix) const;
    QVariant customerId(const QModelIndex& ix) const;
    bool isSameCustomer(const QModelIndexList& mil) const;
    bool hasNoPayment(const QModelIndexList& mil) const;
    bool removeOrders(const QVariantList& ids);
    QVariant paymentId(const QModelIndex&) const;
};

#endif //RelationalSaleModel