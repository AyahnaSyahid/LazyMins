#ifndef FlexibleTableModel_H
#define FlexibleTableModel_H

#include <QSqlTableModel>
#include <QVariant>
#include <QMap>

typedef QVariant (*modelDataAdapter)(const QVariant&, int);

class FlexibleTableModel : public QSqlTableModel {

public:
    FlexibleTableModel(QObject* parent = nullptr) : m_mda(), QSqlTableModel(parent) {}

    ~FlexibleTableModel() {}
    
    QVariant data(const QModelIndex& mi, int role) const override {
        if(!mi.isValid()) return QVariant();
        if(m_mda.contains(mi.column())) {
            return m_mda[mi.column()](QSqlTableModel::data(mi, role), role); 
        }
        return QSqlTableModel::data(mi, role);
    };

    void setColumnAdapter(int c, modelDataAdapter mda) {
        m_mda[c] = mda;
    };

private:
    QMap<int, modelDataAdapter> m_mda;
};

#endif