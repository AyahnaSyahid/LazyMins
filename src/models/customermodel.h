#ifndef CustomerModelH
#define CustomerModelH

#include <QSqlTableModel>

class CustomerModel : public QSqlTableModel
{
    Q_OBJECT
public:
    CustomerModel(QObject* parent=nullptr);
    ~CustomerModel();
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const override;
};

#endif