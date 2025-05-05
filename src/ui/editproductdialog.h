#ifndef EditProductDialog_H
#define EditProductDialog_H

#include "createproductdialog.h"

class EditProductDialog : public CreateProductDialog {
    Q_OBJECT

public:
    explicit EditProductDialog(int, QWidget* =nullptr);
    ~EditProductDialog();

private slots:
    void on_pushButton_clicked() override;

private:
    int _pid;
};

#endif