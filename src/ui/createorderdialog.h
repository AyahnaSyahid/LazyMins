#ifndef CreateOrderDialog_H
#define CreateOrderDialog_H

#include <QDialog>

namespace Ui {
    class CreateOrderDialog;
}

class CreateOrderDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateOrderDialog(QWidget* = nullptr);
    ~CreateOrderDialog();

protected:
    Ui::CreateOrderDialog* ui;
};

#endif // CreateOrderDialog_H