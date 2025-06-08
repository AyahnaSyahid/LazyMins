#ifndef CreateOrderModel_H
#define CreateOrderModel_H

#include <QIdentityProxyModel>

class QSqlQueryModel;
class CreateOrderModel : public  QIdentityProxyModel {
    
    Q_OBJECT
    
public:
    CreateOrderModel(QObject* =nullptr);
    ~CreateOrderModel();
    
    QVariant data(const QModelIndex&, int role=Qt::DisplayRole) const;
    qint64 sum() const;
    qint64 sum(const QList<QModelIndex>&) const;
	void loadAll();
	
public slots:
    void setCustomerId(int);
    void reload();

signals:
    void reloaded();

private:
    QSqlQueryModel* relModel;
};

#endif