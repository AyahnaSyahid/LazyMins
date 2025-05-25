#ifndef CreateAccountDialog_H
#define CreateAccountDialog_H

#include <QDialog>

namespace Ui {
    class CreateAccountDialog;
}

class Database;
class UserManager;
class CreateAccountDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateAccountDialog(Database* _d, QWidget* =nullptr);
    ~CreateAccountDialog();

public slots:
    void on_saveButton_clicked();

private:
    Ui::CreateAccountDialog* ui;
    Database* db;
};

#endif