#ifndef CreateProductDialog_H
#define CreateProductDialog_H

#include <QDialog>

namespace Ui {
    class CreateProductDialog;
}

class CreateProductDialog : public QDialog {
    Q_OBJECT
public:
    explicit CreateProductDialog(QWidget* = nullptr);
    ~CreateProductDialog();

protected slots:
    virtual void on_pushButton_clicked();

protected:
    Ui::CreateProductDialog* ui;
};

#endif