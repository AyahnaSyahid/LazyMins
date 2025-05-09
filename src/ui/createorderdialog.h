#ifndef CreateOrderDialog_H
#define CreateOrderDialog_H

#include <QDialog>


class QDate;
namespace Ui {
    class CreateOrderDialog;
}

class CreateOrderDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateOrderDialog(QWidget* = nullptr);
    ~CreateOrderDialog();

public slots:
    void setCurrentOrderDate(const QDate&);

private slots:
    void on_pickDate_clicked();
    void onCustomerChanged(int);
    void changeProduct(int);
    void updateSubTotal();
    void on_resetButton_clicked();
    
protected:
    Ui::CreateOrderDialog* ui;
};

#endif // CreateOrderDialog_H