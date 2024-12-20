#ifndef ProdukModelH
#define ProdukModelH

#include <QSqlTableModel>

class ProdukModel : public QSqlTableModel
{
    Q_OBJECT
  public:
    ProdukModel(QObject* =nullptr);
    ~ProdukModel();
    QVariant data(const QModelIndex&, int role=Qt::DisplayRole) const override;

  public slots:
    bool addProduct(const QString& kode, 
                    const QString& name, 
                    const QString& qr=QString(),
                    const qreal cost=0 ,
                    const qreal price=0,
                    const qreal area=0,
                    const QString& desc=QString());
    
  signals:
    void produkBaru(const QVariant& pid);
    void produkUpdate(const QVariant& pid);
  
  private:
    using QSqlTableModel::setTable;
};

#endif //ProdukModelH