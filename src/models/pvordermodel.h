#ifndef PVOrderModelH
#define PVOrderModelH

#include <QSqlQueryModel>
#include <QtDebug>

class PV_OrderModel : public QSqlQueryModel {
	Q_OBJECT
    QList<qint64> marked_id;
    qint64 customer_id;
public:
    PV_OrderModel(QObject *parent=nullptr);
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const override;
    void refreshModel();
    inline void unmark(qint64 n) {
        // qDebug() << " Searched :" << n; 
        auto ion = marked_id.indexOf(n);
        if( ion > -1) {
            marked_id.removeAt(ion);
            refreshModel();
        }
    }
    inline void mark(qint64 n) {
        if(marked_id.indexOf(n) < 0) {
            marked_id << n;
            refreshModel();
        }
    }
    
    inline void marks(QList<qint64> ns) {
        if(ns.isEmpty()) return;
        for(auto i=ns.begin(); i!=ns.end(); ++i) {
            marked_id.push_back((*i));
            // qDebug() << " Added :" << *i; 
        }
        refreshModel();
    }
    
    inline void resetMark() {
        marked_id.clear();
        refreshModel();
    }
    void setCustomer(qint64);
};

#endif