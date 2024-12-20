#ifndef PaymentDialog_H
#define PaymentDialog_H

namespace Ui {
    class PaymentDialog;
}

#include <QDialog>

class QAbstractItemModel;
class PaymentDialog : public QDialog {
    Q_OBJECT
    qint64 _cusid;
public:
    PaymentDialog(QAbstractItemModel* t, QWidget* parent=nullptr);
    ~PaymentDialog();
    inline void setCustomer(qint64 cid) { _cusid = cid; }

protected slots:
    virtual void on_pushButton_clicked();
    virtual void on_pushButton2_clicked();
    void updateKembalian();

protected:
    Ui::PaymentDialog* ui;
    QAbstractItemModel* transModel;
    double tHarga;
};

#endif