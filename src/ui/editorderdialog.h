#ifndef EditOrderDialog_H
#define EditOrderDialog_H

#include "createorderdialog.h"

class EditOrderDialog : public CreateOrderDialog {
    Q_OBJECT

public:
    explicit EditOrderDialog(int oid, QWidget* =nullptr);
    ~EditOrderDialog();

private slots:
    void on_pushButton_clicked();

private:
    int _oid;
};

#endif