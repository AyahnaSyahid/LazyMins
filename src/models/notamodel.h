#ifndef NotaModel_H
#define NotaModel_H

#include <QAbstractTableModel>
#include <QVector>
#include <QSqlRecord>

class NotaModel : public QAbstractTableModel {
    Q_OBJECT

protected:
    QVector<QSqlRecord> pjs;

public:
    explicit NotaModel(QObject* parent=nullptr);
    ~NotaModel();
    
    inline int rowCount(const QModelIndex& parent=QModelIndex()) const override {
        if(parent.isValid()) return 0;
        return pjs.count();
    }
    
    inline int columnCount(const QModelIndex& parent=QModelIndex()) const override {
        if(parent.isValid()) return 0;
        if(!pjs.count()) return 0;
        return pjs[0].count();
    }
    
    QVariant headerData(int section, Qt::Orientation orient = Qt::Horizontal, int role = Qt::DisplayRole) const override;
    
    QVariant data(const QModelIndex& mi, int role = Qt::DisplayRole) const override;
    bool hasRecord(const QSqlRecord& rc) const;
    
    inline QVector<QSqlRecord> records() const { return pjs; }
    
    inline void clear() {
        beginResetModel();
        pjs.clear();
        endResetModel();
    }
    
    
    
public slots:
    void pushRecord(const QSqlRecord& pj);
    void pushRecord(const QVector<QSqlRecord>& vpj);
    inline QSqlRecord pullRecord(const QModelIndex& ix) { return pullRecord(ix.column()); }
    QSqlRecord pullRecord(int row);

};

#endif