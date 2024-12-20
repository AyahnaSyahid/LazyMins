#ifndef RepaymentModelH
#define RepaymentModelH

#include <QSqlTableModel>

class RepaymentModel : public QSqlTableModel
{
    Q_OBJECT
    
  public:
    RepaymentModel(QObject* =nullptr);
    ~RepaymentModel();
    QVariant data(const QModelIndex&, int = Qt::DisplayRole) const;
    void setEditable(bool);
    
    Qt::ItemFlags flags(const QModelIndex& i) const;
    static bool insertRepayment(int paymentId, int value, bool trf=false);
    
  private:
    bool allowEdit;
    using QSqlTableModel::setTable;
};

#endif //RepaymentModelH