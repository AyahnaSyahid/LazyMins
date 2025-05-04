#ifndef EditCustomerDialog_H
#define EditCustomerDialog_H

#include "createcustomerdialog.h"

class EditCustomerDialog : public CreateCustomerDialog {
    Q_OBJECT
    
public:
    EditCustomerDialog(int cid, QWidget* = nullptr);
    ~EditCustomerDialog();

private slots:
    void on_pushButton_clicked();

private:
    int customerId;
};

#endif