#ifndef AddCustomerDialog_H
#define AddCustomerDialog_H

namespace Ui {
    class AddCustomerDialog;
}

#include <QDialog>

class AddCustomerDialog : public QDialog {
    Q_OBJECT
public:
    AddCustomerDialog(QWidget* = nullptr);
    ~AddCustomerDialog();

// public slots:
    // void accept() override;

private slots:
    void on_pushButton_clicked();
    
signals:
    void customerAdded();

private:
    bool saveSucess;
    Ui::AddCustomerDialog* ui;
};

#endif