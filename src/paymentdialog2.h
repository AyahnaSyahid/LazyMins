#ifndef PaymentDialog2H
#define PaymentDialog2H

#include <QDialog>

namespace Ui {
    class PaymentDialog;
}

class PaymentDialog2 : public QDialog
{
    Q_OBJECT
  public:
    PaymentDialog2(QWidget * =nullptr);
    ~PaymentDialog2();
    void setPayment(qint64);

  private slots:
    void on_pushButton2_clicked();
    void on_doubleSpinBox2_valueChanged(qreal);

  private:
    Ui::PaymentDialog *ui;
    qint64 inv_id;
};

#endif //PaymentDialog2H