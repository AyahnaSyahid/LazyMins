#ifndef CreateCustomerDialog_H
#define CreateCustomerDialog_H

#include <QDialog>

namespace Ui {
    class CreateCustomerDialog;
}

class CreateCustomerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCustomerDialog(QWidget* =nullptr);
    ~CreateCustomerDialog();

private slots:
    void on_pushButton_clicked();
    void reset_input();

protected:
    Ui::CreateCustomerDialog* ui;

signals:
    void newCustomer(const QVariant& lii);
};

#endif // AddCustomerDialog_H