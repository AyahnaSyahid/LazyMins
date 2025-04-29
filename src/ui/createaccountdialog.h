#ifndef CreateAccountDialog_H
#define CreateAccountDialog_H

#include <QDialog>

namespace Ui {
    class CreateAccountDialog;
}

class CreateAccountDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateAccountDialog(QWidget* =nullptr);
    ~CreateAccountDialog();

public slots:
    void on_saveButton_clicked();

private:
    Ui::CreateAccountDialog* ui;
};

#endif