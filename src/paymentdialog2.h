#ifndef PaymentDialog2H
#define PaymentDialog2H

#include "paymentdialog.h"

class PaymentDialog2 : public PaymentDialog
{
    Q_OBJECT
  public:
    PaymentDialog2(QAbstractItemModel *mod, QWidget* =nullptr);

  protected slots:
    void on_pushButton_clicked() Q_DECL_OVERRIDE;
    void on_pushButton2_clicked() Q_DECL_OVERRIDE;
};

#endif //PaymentDialog2H