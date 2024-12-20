#ifndef TabelRepaymentH
#define TabelRepaymentH

#include <QWidget>

namespace Ui {
    class TabelRepayment;
}

class QAbstractItemModel;
class TabelRepayment : public QWidget
{
    Q_OBJECT
  public:
    TabelRepayment(QWidget* =nullptr);
    ~TabelRepayment();
    void setNota(qint64);
  
  private slots:
    void on_tableView_doubleClicked(const QModelIndex&);
    void on_pushButton_clicked();
    void on_pushButton2_clicked();
    void on_pushButton3_clicked();
    void initRepayModel();
    void onAcceptedRepayment();
    
  private:
    Ui::TabelRepayment* ui;
};

#endif //TabelRepaymentH

