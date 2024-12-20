#ifndef ModelPembayaranNotaH
#define ModelPembayaranNotaH

#include <QSqlQueryModel>

class ModelPembayaranNota : public QSqlQueryModel
{
    Q_OBJECT
    
public:
    static QString defaultQuery;
    
    ModelPembayaranNota(QObject* parent=nullptr);
    ~ModelPembayaranNota();
    QVariant data(const QModelIndex& ix, int role=Qt::DisplayRole) const override;
    QString generatedQuery() const;

public slots:
    void lunasToggle(bool);
    void orderToggle(bool);
    void setFilterName(const QString& n);
    void setFilterId(const QString& sid);

private:
    bool showLunas;
    bool orderDesc;
    QString filterName;
    QString filterId;
};

#endif