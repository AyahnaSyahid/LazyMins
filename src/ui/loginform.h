#ifndef LoginForm_H
#define LoginForm_H

#include <QDialog>

namespace Ui {
    class LoginForm;
}

class UserManager;
class LoginForm : public QDialog {
    Q_OBJECT

public:
    explicit LoginForm(UserManager* man, QWidget* = nullptr);
    ~LoginForm();

public slots:
    void setAccountName(const QString&);

private slots:
    void on_pushButton_clicked();

private:
    UserManager* uman;
    Ui::LoginForm* ui;
};

#endif