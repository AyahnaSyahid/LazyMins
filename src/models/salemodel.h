#ifndef SaleModelH
#define SaleModelH

#include <QSqlTableModel>

class SaleModel : public QSqlTableModel
{
    Q_OBJECT
  public:
    SaleModel(QObject* =nullptr);
    ~SaleModel();
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const override;
};

#endif //SaleModelH